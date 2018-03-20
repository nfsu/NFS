#pragma once

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qlayout.h>
#include "boxedstruct.h"

namespace nfsu {

	class InfoWindow : public QWidget {

	public:

		InfoWindow(std::string str, QWidget *parent = nullptr);
		~InfoWindow();

		void clear();
		void addString(QString text);

		template<typename ...args>
		void display(TBoxedStruct<args...> &what, std::string(&names)[nfs::CountArgs<args...>::get()]);

	private:

		Buffer displayBuffer;
		QLayout *layout = nullptr;

	};
	
	template<typename T>
	struct DisplayInter {
    
		static void run(T &what, InfoWindow *which, u32 *count, std::string *names) {
			++*count;
		}
    
	};
    
	template<>
	struct DisplayInter<u32> {
    
		static void run(u32 &what, InfoWindow *which, u32 *count, std::string *names) {
    
			u32 &countr = *count;
			std::string &str = names[countr];
    
			QString rep = QString(str.c_str()).replace("%u", QString::number(what));
    
			which->addString(rep);
			++countr;
    
		}
    
	};
    
	template<>
	struct DisplayInter<std::string> {
    
		static void run(std::string &what, InfoWindow *which, u32 *count, std::string *names) {
    
			u32 &countr = *count;
			std::string &str = names[countr];
    
			QString rep = QString(str.c_str()).replace("%s", what.c_str());
    
			which->addString(rep);
			++countr;
		}
    
	};
    
	template<>
	struct DisplayInter<u8*> {
    
		static void run(u8 *&what, InfoWindow *which, u32 *count, std::string *names) {
    
			u32 &countr = *count;
			std::string &str = names[countr];
    
			QString rep = QString(str.c_str()).replace("%p", QString::number((size_t)what, 16));
    
			which->addString(rep);
			++countr;
		}
    
	};

	template<typename ...args>
	void InfoWindow::display(TBoxedStruct<args...> &what, std::string (&names)[nfs::CountArgs<args...>::get()]) {

		layout = new QVBoxLayout(this);
		u32 count = 0;
		what.template run<DisplayInter>(what, this, &count, names);

	}

}