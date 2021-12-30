#pragma once

#include <string>

namespace GameGuy {

	class Panel {
	public:
		Panel(const std::string& name);
		~Panel();

		void open();
		void render();
		void close();

		inline bool isOpen() const { return mOpen; }
	protected:
		virtual void onImGuiRender() = 0;
	protected:
		bool mOpen;
		std::string mName;
	};

}
