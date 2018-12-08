#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/checkbox.hpp>
#include <string>
#include <vector>
#include <mutex>

int add(int, int);

using namespace nana;

//Creates a textbox and button
//textbox shows the value of the sub item
//button is used to delete the item.
class inline_widget : public listbox::inline_notifier_interface {
private:
	//Creates inline widget
	//listbox calls this method to create the widget
	//The position and size of widget can be ignored in this process
	virtual void create(window wd) override {
		//Create listbox
		txt_.create(wd);
		txt_.events().click([this] {
			//Select the item when clicks the textbox
			indicator_->selected(pos_);
		});

		txt_.events().mouse_move([this] {
			//Highlight the item when hovers the textbox
			indicator_->hovered(pos_);
		});

		txt_.events().key_char([this](const arg_keyboard& arg) {
			if (arg.key == keyboard::enter) {
				//Modify the item when enter is pressed
				arg.ignore = true;
				indicator_->modify(pos_, txt_.caption());
			}
		});
		//Or modify the item when typing
		txt_.events().text_changed([this]() {
			indicator_->modify(pos_, txt_.caption());
		});
		//Create button
		btn_.create(wd);
		btn_.caption("Delete");
		btn_.events().click([this] {
			//Delete the item when button is clicked
			auto & lsbox = dynamic_cast<listbox&>(indicator_->host());
			lsbox.erase(lsbox.at(pos_));
		});

		btn_.events().mouse_move([this] {
			//Highlight the item when hovers the button
			indicator_->hovered(pos_);
		});
	}

	//Activates the inline widget, bound to a certain item of the listbox
	//The inline_indicator is an object to operate the listbox item,
	//pos is an index object refers to the item of listbox
	virtual void activate(inline_indicator& ind, index_type pos) {
		indicator_ = &ind;
		pos_ = pos;
	}

	void notify_status(status_type status, bool status_on) override {
		//Sets focus for the textbox when the item is selected
		if ((status_type::selecting == status) && status_on)
			txt_.focus();
	}

	//Sets the inline widget size
	//dimension represents the max size can be set
	//The coordinate of inline widget is a logical coordinate to the sub item of listbox
	void resize(const size& dimension) override {
		auto sz = dimension;
		sz.width -= (sz.width < 50 ? 0 : 50); //Check for avoiding underflow.
		txt_.size(sz);

		rectangle r(sz.width + 5, 0, 45, sz.height);
		btn_.move(r);
		//rectangle t(sz.width, 5, 100, sz.height);
		//chb_.move(t);
	}

	//Sets the value of inline widget with the value of the sub item
	virtual void set(const value_type& value) {
		//Avoid emitting text_changed to set caption again, otherwise it
		//causes infinite recursion.
		if (txt_.caption() != value)
			txt_.caption(value);
	}

	//Determines whether to draw the value of sub item
	//e.g, when the inline widgets covers the whole background of the sub item,
	//it should return false to avoid listbox useless drawing
	bool whether_to_draw() const override {
		return false;
	}
private:
	inline_indicator * indicator_{ nullptr };
	index_type pos_;
	textbox txt_;
	button btn_;
};

struct data {
	char* username;
	char* password;
};
int main() {
	std::vector<data> datas;

	////Define a form.
	form fm;
	fm.caption("Password Manager");

	//Then append items
	datas.push_back(data{ (char *)"Hello0", (char *)"World0" });
	datas.push_back(data{ (char *)"Hello1", (char *)"World1" });
	datas.push_back(data{ (char *)"Hello2", (char *)"World2" });
	datas.push_back(data{ (char *)"Hello3", (char *)"World3" });

	auto value_translator = [](const std::vector<nana::listbox::cell>& cells) {
		data p;
		char * u = (char*)malloc(cells[0].text.size());
		int u_len = cells[0].text.size();
		auto sou = cells[0].text.c_str();
		__asm {
			mov ecx, u_len
			lea esi, u
			lea edi, sou
			rep movsb 
		}
		char * ps = (char*)malloc(cells[1].text.size());
		//u = (char*)malloc(cells[1].text.size());
		u_len = cells[1].text.size();
		sou = cells[1].text.c_str();
		__asm {
			mov ecx, u_len
			lea esi, ps
			lea edi, sou
			rep movsb

		}
		p.username = u;
		p.password = ps;//std::stoul(cells[1].text);
		return p;
	};

	auto cell_translator = [](const data& p) {
		std::vector<nana::listbox::cell> cells;
		cells.emplace_back(p.username);
		cells.emplace_back(p.password);
		return cells;
	};

	////Define a label and display a text.
	//label lab{ fm, "Hello, <bold blue size=16>Nana C++ Library</>" };
	//lab.format(true);

	//Define a button and answer the click event.
	button qut{ fm, "Quit" };
	qut.events().click([&fm] {
		fm.close();
	});
	
	button add{ fm, "Add" };
	add.events().click([&fm] {
		fm.close();
	});


	////Show the form
	//fm.show();

	////Start to event loop process, it blocks until the form is closed.
	//exec();
	//lab.caption(std::string("Ali"));
	//fm.show();
	//form fm;
	listbox lsbox(fm, rectangle{ 0, 0, 500, 400 });

	//Create two columns
	lsbox.checkable(1);
	lsbox.append_header("Username");
	lsbox.append_header("Password");
	lsbox.at(0).model<std::recursive_mutex>(datas, value_translator, cell_translator);
	//lsbox.events.checked([](listbox & lsbox) {
	//	auto a = lsbox.at(0);
	//	for (auto & n: a) {
	//		//n.text()
	//	}
	//}(lsbox));




	//Set the inline_widget, the first column of category 0, the second column of category 1
	lsbox.at(0).inline_factory(0, pat::make_factory<inline_widget>());
	//Layout management
	//fm.div("vert <><<><weight=80% text><>><><weight=24<><button><>><>");
	fm.div("< <vert <weight=80%<weight=80% text>| <vert <><add><> >> |<button>> > ");
	fm["text"] << lsbox;
	fm["button"] << qut;
	fm["add"] << add;
	fm.collocate();

	fm.show();
	exec();
}

int add(int, int) {
	__asm
	{
		mov   eax, [esp + 4]; argument 1
		add   eax, [esp + 8]; argument 2
	}
}

