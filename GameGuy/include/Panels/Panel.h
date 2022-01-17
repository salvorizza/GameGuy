#pragma once

#include <string>

namespace GameGuy {

	class Panel {
	public:
		Panel(const std::string& name, bool hasMenuBar, bool noPadding = false, bool defaultOpen = true);
		~Panel();

		void open();
		void render();
		void close();

		inline bool isOpen() const { return mOpen; }
	protected:
		virtual void onImGuiRender() = 0;
	protected:
		bool mOpen,mHasMenuBar,mNoPadding;
		std::string mName;
	};

}
