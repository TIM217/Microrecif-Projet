// Antoine Bernadat, Timothy Biersohn
#ifndef GUI_H
#define GUI_H

#include <string>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/fixed.h>
#include <gtkmm/separator.h>
#include <gtkmm/frame.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/checkbutton.h>

struct Frame // Cadrage du modèle et dimensions du dessin
{
	double xMin; // Bornes du modèle
	double xMax;
	double yMin;
	double yMax;
	double asp;  // Rapport largeur / hauteur
	int width;   // Largeur du dessin
	int height;  // Hauteur du dessin
};

class MyArea : public Gtk::DrawingArea
{
public:
	MyArea();
	virtual ~MyArea();

	void setFrame(Frame x); 
	void adjustFrame(int width, int height);

	void clear();
	void draw();

protected:
	void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);

private:
	Frame frame;
};

class Gui : public Gtk::Window
{
public:
	Gui();
	virtual ~Gui();
	static void set_file(std::string file_name);
	void set_box();
	void set_label();
	void update_label();

private:
	void on_button_clicked_exit();
	void on_button_clicked_open();
	void on_button_clicked_save();
	void on_button_clicked_start();
	bool on_timeout();
	void on_button_clicked_step();
	void on_button_toggled_cocher();

	void on_file_dialog_response_open(int response_id, Gtk::FileChooserDialog* dialog);
	void on_file_dialog_response_save(int response_id, Gtk::FileChooserDialog* dialog);

	bool on_window_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);

	Gtk::Frame m_Frame1, m_Frame2;

	Gtk::Box m_Main_Box, m_Draw_Box, m_Double_Box, m_Label_Box, m_Buttons_Box, 
	         m_Title_Box, m_Count_Box;

	Gtk::Separator m_Separator;

	Gtk::Button m_Button_Exit, m_Button_Open, m_Button_Save, m_Button_Start, 
				m_Button_Step;

	Gtk::CheckButton m_Button_Cocher;

	Gtk::Label m_Label_General, m_Label_Info, m_Label_Maj, 
			   m_Label_Algues, m_Label_Coraux, m_Label_Scavengers,
			   m_Label_maj, m_Label_alg, m_Label_cor, m_Label_sca;

	MyArea m_Area;

	bool naissance_active = false;
	sigc::connection timer; 
	const int timeout_value;
	int maj, start_stop;
};

#endif
