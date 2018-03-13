#pragma once

#include <QtWidgets/qwidget.h>
#include "boxedstruct.h"

namespace nfsu {

	class InfoWindow : public QWidget {

	public:

		InfoWindow() {}
		~InfoWindow() {}

		template<typename ...args>
		void display(TBoxedStruct<args...> what);

	protected:

		template<typename T>
		struct DisplayInter {

			static void run(const InfoWindow *which) {

				printf("%s\n", typeid(T).name());

			}

		};

	private:

		Buffer displayBuffer;

	};

	template<typename ...args>
	void InfoWindow::display(TBoxedStruct<args...> what) {

		what.run<DisplayInter>(this);

	}

}