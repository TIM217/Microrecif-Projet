# Makefile — Simulation d'écosystème marin (BA2 INFO-F-106)
# Auteurs : Antoine Bernadat, Timothy Biersohn
#
# Compatible Linux (apt) et macOS (Homebrew sur Apple Silicon / Intel)

OUT      = projet
CXX      = g++
CXXFLAGS = -Wall -std=c++17

# --- Détection automatique de pkg-config (macOS/Homebrew + Linux) ---
# Sur macOS avec Homebrew, on construit le PKG_CONFIG_PATH complet :
#   - lib/pkgconfig et share/pkgconfig du prefix Homebrew
#   - le dossier OS interne de Homebrew (nécessaire pour bzip2, etc.)
BREW_PREFIX := $(shell \
	if ! pkg-config --exists gtkmm-4.0 2>/dev/null; then \
		if [ -d /opt/homebrew ]; then echo "/opt/homebrew"; \
		elif [ -d /usr/local ]; then echo "/usr/local"; \
		fi; \
	fi)

MACOS_MAJOR := $(shell sw_vers -productVersion 2>/dev/null | cut -d. -f1)
BREW_OS_PKG := $(BREW_PREFIX)/Library/Homebrew/os/mac/pkgconfig/$(MACOS_MAJOR)

PKG_CONFIG_PATH_EXTRA := $(BREW_OS_PKG):$(BREW_PREFIX)/lib/pkgconfig:$(BREW_PREFIX)/share/pkgconfig
PKG_CONFIG := PKG_CONFIG_PATH="$(PKG_CONFIG_PATH_EXTRA):$$PKG_CONFIG_PATH" pkg-config

LINKING  = `$(PKG_CONFIG) --cflags gtkmm-4.0`
LDLIBS   = `$(PKG_CONFIG) --libs gtkmm-4.0`

CXXFILES = graphic.cc shape.cc lifeform.cc simulation.cc projet.cc gui.cc message.cc
OFILES   = $(CXXFILES:.cc=.o)

.PHONY: all clean depend

all: $(OUT)

$(OUT): $(OFILES)
	$(CXX) $(CXXFLAGS) $(LINKING) $(OFILES) -o $@ $(LDLIBS)

clean:
	@echo " *** Suppression des modules objet et de l'exécutable ***"
	@/bin/rm -f *.o *.x *.cc~ *.h~ $(OUT)

depend:
	@echo " *** Mise à jour des dépendances ***"
	@(sed '/^# DO NOT DELETE THIS LINE/q' Makefile && \
	  $(CXX) -MM $(CXXFLAGS) $(LINKING) $(CXXFILES) | \
	  egrep -v "/usr" \
	 ) > Makefile.new
	@mv Makefile.new Makefile

# Règles de compilation individuelles
graphic.o: graphic.cc graphic_gui.h graphic.h
	$(CXX) $(CXXFLAGS) $(LINKING) -c $< -o $@

gui.o: gui.cc graphic_gui.h graphic.h gui.h simulation.h lifeform.h shape.h constantes.h
	$(CXX) $(CXXFLAGS) $(LINKING) -c $< -o $@

projet.o: projet.cc gui.h simulation.h lifeform.h message.h shape.h graphic.h constantes.h
	$(CXX) $(CXXFLAGS) $(LINKING) -c $< -o $@

# --- Dépendances générées automatiquement ---
# DO NOT DELETE THIS LINE
shape.o: shape.cc shape.h graphic.h
lifeform.o: lifeform.cc message.h lifeform.h constantes.h shape.h graphic.h
simulation.o: simulation.cc message.h simulation.h lifeform.h constantes.h shape.h graphic.h
message.o: message.cc message.h

# Tests du modèle sans ouvrir de fenêtre ; les fichiers fournis restent inchangés.
.PHONY: test test-sanitize
MODEL_SOURCES = tests/runner/model.cc simulation.cc lifeform.cc shape.cc message.cc
MODEL_HEADERS = simulation.h lifeform.h shape.h constantes.h message.h graphic.h

tests/runner/model: $(MODEL_SOURCES) $(MODEL_HEADERS)
	$(CXX) $(CXXFLAGS) -O1 -I. $(MODEL_SOURCES) -o $@

test: tests/runner/model
	python3 tests/runner/regression.py ./tests/runner/model

test-sanitize:
	$(CXX) $(CXXFLAGS) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -I. $(MODEL_SOURCES) -o tests/runner/model-sanitize
	ASAN_OPTIONS=detect_leaks=0:symbolize=0 UBSAN_OPTIONS=halt_on_error=1:symbolize=0 python3 tests/runner/regression.py ./tests/runner/model-sanitize

.PHONY: demo
demo: $(OUT)
	./$(OUT) examples/recif.txt
