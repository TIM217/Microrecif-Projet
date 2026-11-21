// Antoine Bernadat, Timothy Biersohn
#include <iostream>
#include <string>
#include <glibmm/main.h>
#include "gui.h"
#include "graphic_gui.h"
#include "simulation.h"

constexpr double max(256.);	// Dimension du monde
constexpr int taille_dessin(500);
constexpr double speed_time_ms(10); // Délai entre deux pas, en millisecondes
constexpr int window_size_h(800); // Largeur initiale de la fenêtre
constexpr int window_size_v(600); // Hauteur initiale de la fenêtre

static void orthographic_projection(const Cairo::RefPtr<Cairo::Context> &cr,
									const Frame &frame);
static void draw_frame(const Cairo::RefPtr<Cairo::Context> &cr, Frame frame);
// Cadrage initial du modèle.
static Frame default_frame = {0., max, 0., max, 1., taille_dessin, taille_dessin};


static Simulation sim;

Gui::Gui() : m_Main_Box(Gtk::Orientation::HORIZONTAL, 0),
			 m_Draw_Box(Gtk::Orientation::VERTICAL, 0),
			 m_Double_Box(Gtk::Orientation::VERTICAL, 0),
			 m_Label_Box(Gtk::Orientation::HORIZONTAL, 2),
			 m_Buttons_Box(Gtk::Orientation::VERTICAL, 2),
			 m_Separator(Gtk::Orientation::VERTICAL),
			 m_Button_Exit("Exit"),
			 m_Button_Open("Open"),
			 m_Button_Save("Save"),
			 m_Button_Start("Start"),
			 m_Button_Step("Step"),
			 m_Button_Cocher("Naissance d'algue"),
			 m_Label_General("\nGeneral \n"),
			 m_Label_Info("\n Info : nombre de... \n"),
			 m_Label_Maj(" mise à jour : \n"),
			 m_Label_Algues(" algues : \n"),
			 m_Label_Coraux(" corails : \n"),
			 m_Label_Scavengers(" charognards : \n"),
			 timeout_value(speed_time_ms)
{
	maj = 0;
	start_stop = 0;
	set_title("Microrécif");
	set_default_size(window_size_h, window_size_v);
	set_resizable(true);
	set_child(m_Main_Box);

	set_box();
	set_label();
	update_label();

	m_Area.set_size_request(taille_dessin, taille_dessin);

	m_Button_Exit.signal_clicked().connect(
		sigc::mem_fun(*this, &Gui::on_button_clicked_exit));

	m_Button_Open.signal_clicked().connect(
		sigc::mem_fun(*this, &Gui::on_button_clicked_open));

	m_Button_Save.signal_clicked().connect(
		sigc::mem_fun(*this, &Gui::on_button_clicked_save));

	m_Button_Start.signal_clicked().connect(
		sigc::mem_fun(*this, &Gui::on_button_clicked_start));

	m_Button_Step.signal_clicked().connect(
		sigc::mem_fun(*this, &Gui::on_button_clicked_step));

	m_Button_Cocher.signal_toggled().connect(
		sigc::mem_fun(*this, &Gui::on_button_toggled_cocher));

	auto controller = Gtk::EventControllerKey::create();
	controller->signal_key_pressed().connect(
		sigc::mem_fun(*this, &Gui::on_window_key_pressed), false);
	add_controller(controller);
}

Gui::~Gui()
{ timer.disconnect(); }

void Gui::on_button_clicked_exit()
{
	std::cout << "Exit" << std::endl;
	timer.disconnect();
	close();
}

// Ouvre le sélecteur de fichier après mise en pause.
void Gui::on_button_clicked_open()
{
	if (start_stop % 2 != 0) on_button_clicked_start();
	std::cout << "Open" << std::endl;

	auto dialog = new Gtk::FileChooserDialog("Please choose a file",
											 Gtk::FileChooser::Action::OPEN);
	dialog->set_transient_for(*this);
	dialog->set_modal(true);
	dialog->signal_response().connect(sigc::bind(
		sigc::mem_fun(*this, &Gui::on_file_dialog_response_open), dialog));

	
	dialog->add_button("_Cancel", Gtk::ResponseType::CANCEL);
	dialog->add_button("_Open", Gtk::ResponseType::OK);

	auto filter_text = Gtk::FileFilter::create();
	filter_text->set_name("Text files");
	filter_text->add_mime_type("text/plain");
	dialog->add_filter(filter_text);

	
	dialog->show();
}

void Gui::on_file_dialog_response_open(int response_id,
									   Gtk::FileChooserDialog *dialog)
{
	
	switch (response_id)
	{
	case Gtk::ResponseType::OK:
	{
		std::cout << "Open clicked." << std::endl;

		auto selected = dialog->get_file();
		if (!selected) break;
		auto file_name = selected->get_path();
		std::cout << "File selected: " << file_name << std::endl;
        sim.destroy();
		maj = 0;
		set_file(file_name);
		update_label();
		m_Area.queue_draw();
		break;
	}
	case Gtk::ResponseType::CANCEL:
	{
		std::cout << "Cancel clicked." << std::endl;
		break;
	}
	}
	delete dialog;
}

void Gui::on_button_clicked_save()
{
	if (start_stop % 2 != 0) on_button_clicked_start();
	std::cout << "Save" << std::endl;

	auto dialog = new Gtk::FileChooserDialog("Please choose a file",
											 Gtk::FileChooser::Action::SAVE);
	dialog->set_transient_for(*this);
	dialog->set_modal(true);
	dialog->signal_response().connect(sigc::bind(
		sigc::mem_fun(*this, &Gui::on_file_dialog_response_save), dialog));

	
	dialog->add_button("_Cancel", Gtk::ResponseType::CANCEL);
	dialog->add_button("_Save", Gtk::ResponseType::OK);

	auto filter_text = Gtk::FileFilter::create();
	filter_text->set_name("Text files");
	filter_text->add_mime_type("text/plain");
	dialog->add_filter(filter_text);

	
	dialog->show();
}

void Gui::on_file_dialog_response_save(int response_id,
									   Gtk::FileChooserDialog *dialog)
{
	
	switch (response_id)
	{
	case Gtk::ResponseType::OK:
	{
		std::cout << "Save clicked." << std::endl;

		auto selected = dialog->get_file();
		if (!selected) break;
		auto file_name = selected->get_path();
		std::cout << "File selected: " << file_name << std::endl;
		sim.save(file_name);
		break;
	}
	case Gtk::ResponseType::CANCEL:
	{
		std::cout << "Cancel clicked." << std::endl;
		break;
	}
	}
	delete dialog;
}

void Gui::on_button_clicked_start()
{
	if (start_stop % 2 == 0)
	{
		std::cout << "Start" << std::endl;
		m_Button_Start.set_label("Stop");

		sigc::slot<bool()> my_slot = sigc::bind(
			sigc::mem_fun(*this, &Gui::on_timeout));

		timer = Glib::signal_timeout().connect(my_slot, timeout_value);
	}
	else
	{
		std::cout << "Stop" << std::endl;
		m_Button_Start.set_label("Start");

		timer.disconnect();
	}
	++start_stop;
}

bool Gui::on_timeout()
{
	++maj;

	sim.mise_a_jour(maj, naissance_active);

	m_Area.queue_draw();

	update_label();


	return true;
}

void Gui::on_button_clicked_step()
{
	if (start_stop % 2 == 0)
	{
		++maj;

		sim.mise_a_jour(maj, naissance_active);

		m_Area.queue_draw();	

		update_label();
	}
}

void Gui::on_button_toggled_cocher()
{
	std::cout << "Naissance active : "

			  << (m_Button_Cocher.get_active() ? "true" : "false")

			  << std::endl;

	naissance_active = m_Button_Cocher.get_active();
}

bool Gui::on_window_key_pressed(guint keyval, guint, Gdk::ModifierType)
{
	switch (gdk_keyval_to_unicode(keyval))
	{
	case 's':
		on_button_clicked_start();
		return true;
	case '1':
		on_button_clicked_step();
		return true;
	case 'q':
		on_button_clicked_exit();
		return true;
	}
	
	return false;
}

void Gui::set_file(std::string filename)
{
	sim.lecture(filename);
}

MyArea::MyArea() : frame()
{
	set_content_width(default_frame.width);
	set_content_height(default_frame.height);

	set_draw_func(sigc::mem_fun(*this, &MyArea::on_draw));
}

MyArea::~MyArea()
{
}

void MyArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height)
{
	if (width <= 0 || height <= 0) return;
	cr->save();
	cr->set_source_rgb(1, 1, 1);
	cr->paint();
	// Conserve les proportions du modèle dans la zone de dessin.
	adjustFrame(width, height);

	orthographic_projection(cr, frame); // Passage des coordonnées du modèle aux pixels
	draw_frame(cr, frame);				// Bordure du monde

	set_cr(cr);
	sim.dessin_simu();
	cr->restore();
}

void MyArea::setFrame(Frame f)
{
	if ((f.xMin <= f.xMax) and (f.yMin <= f.yMax) and (f.height > 0))
	{
		f.asp = static_cast<double>(f.width) / f.height;
		frame = f;
	}
	else
		std::cout << "incorrect Model framing or window parameters" << std::endl;
}

void MyArea::adjustFrame(int width, int height)
{
	frame.width = width;
	frame.height = height;

	// Adapte le cadrage au rapport largeur / hauteur disponible.
	

	// Le cadrage de référence reste centré.
	double new_aspect_ratio((double)width / height);
	if (new_aspect_ratio > default_frame.asp)
	{ // Conserve les bornes verticales et élargit les bornes horizontales.
		frame.yMax = default_frame.yMax;
		frame.yMin = default_frame.yMin;

		double delta(default_frame.xMax - default_frame.xMin);
		double mid((default_frame.xMax + default_frame.xMin) / 2);
		// Centre le cadrage sur l’axe horizontal.
		frame.xMax = mid + 0.5 * (new_aspect_ratio / default_frame.asp) * delta;
		frame.xMin = mid - 0.5 * (new_aspect_ratio / default_frame.asp) * delta;
	}
	else
	{ // Conserve les bornes horizontales et élargit les bornes verticales.
		frame.xMax = default_frame.xMax;
		frame.xMin = default_frame.xMin;

		double delta(default_frame.yMax - default_frame.yMin);
		double mid((default_frame.yMax + default_frame.yMin) / 2);
		// Centre le cadrage sur l’axe vertical.
		frame.yMax = mid + 0.5 * (default_frame.asp / new_aspect_ratio) * delta;
		frame.yMin = mid - 0.5 * (default_frame.asp / new_aspect_ratio) * delta;
	}
}

static void orthographic_projection(const Cairo::RefPtr<Cairo::Context> &cr,
									const Frame &frame)
{
	// déplace l'origine au centre de la fenêtre
	cr->translate(frame.width / 2, frame.height / 2);

	// normalise la largeur et hauteur aux valeurs fournies par le cadrage
	// ET inverse la direction de l'axe Y
	cr->scale(frame.width / (frame.xMax - frame.xMin),
			  -frame.height / (frame.yMax - frame.yMin));

	// décalage au centre du cadrage
	cr->translate(-(frame.xMin + frame.xMax) / 2, -(frame.yMin + frame.yMax) / 2);
}

static void draw_frame(const Cairo::RefPtr<Cairo::Context> &cr, Frame frame)
{
	// Dessine la bordure grise du monde.
	cr->set_line_width(2);
	
	cr->set_source_rgb(0.8, 0.8, 0.8);    // Gris clair
	cr->rectangle(0, 0, max, max);
	cr->stroke();
}

void Gui::set_box()
{
	m_Main_Box.append(m_Double_Box);
	m_Main_Box.append(m_Draw_Box);
	m_Double_Box.append(m_Frame1);
	m_Frame1.set_child(m_Buttons_Box);
	m_Buttons_Box.append(m_Label_General);
	m_Buttons_Box.append(m_Separator);
	m_Draw_Box.append(m_Area);

	m_Area.set_size_request(taille_dessin, taille_dessin);
	m_Area.set_expand(true);

	m_Buttons_Box.append(m_Button_Exit);
	m_Buttons_Box.append(m_Button_Open);
	m_Buttons_Box.append(m_Button_Save);
	m_Buttons_Box.append(m_Button_Start);
	m_Buttons_Box.append(m_Button_Step);
	m_Buttons_Box.append(m_Button_Cocher);
}

void Gui::set_label()
{
	m_Double_Box.append(m_Frame2);
	m_Frame2.set_child(m_Label_Box);
	m_Label_Box.append(m_Title_Box);
	m_Title_Box.set_halign(Gtk::Align::START);
	m_Title_Box.append(m_Label_Info);
	m_Label_Info.set_halign(Gtk::Align::START);
	m_Title_Box.append(m_Label_Maj);
	m_Label_Maj.set_halign(Gtk::Align::START);
	m_Title_Box.append(m_Label_Algues);
	m_Label_Algues.set_halign(Gtk::Align::START);
	m_Title_Box.append(m_Label_Coraux);
	m_Label_Coraux.set_halign(Gtk::Align::START);
	m_Title_Box.append(m_Label_Scavengers);
	m_Label_Scavengers.set_halign(Gtk::Align::START);

	m_Label_Box.append(m_Count_Box);
	m_Count_Box.set_halign(Gtk::Align::START);
	m_Count_Box.set_valign(Gtk::Align::START);
	m_Count_Box.append(m_Label_maj);
	m_Label_maj.set_halign(Gtk::Align::END);
	m_Count_Box.append(m_Label_alg);
	m_Label_alg.set_halign(Gtk::Align::END);
	m_Count_Box.append(m_Label_cor);
	m_Label_cor.set_halign(Gtk::Align::END);
	m_Count_Box.append(m_Label_sca);
	m_Label_sca.set_halign(Gtk::Align::END);

	m_Title_Box.set_orientation(Gtk::Orientation::VERTICAL);
	m_Count_Box.set_orientation(Gtk::Orientation::VERTICAL);

	// Réserver la même largeur, même lorsque les compteurs changent de chiffres.
	// Sinon GTK redimensionne le panneau et décale le cadrage du dessin à chaque pas.
	for (auto* compteur : {&m_Label_maj, &m_Label_alg, &m_Label_cor, &m_Label_sca})
	{
		compteur->set_width_chars(10);
		compteur->set_max_width_chars(10);
		compteur->set_halign(Gtk::Align::FILL);
		compteur->set_xalign(1.0f);
		compteur->set_hexpand(false);
	}
	m_Double_Box.set_hexpand(false);
}

void Gui::update_label()
{
	m_Label_maj.set_text("\n \n \n" + std::to_string(maj) + "\n");
	m_Label_alg.set_text(std::to_string(sim.get_nb_algue()) + "\n");
	m_Label_cor.set_text(std::to_string(sim.get_nb_corail()) + "\n");
	m_Label_sca.set_text(std::to_string(sim.get_nb_scavenger()) + "\n");
}
