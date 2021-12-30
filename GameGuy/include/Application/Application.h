#pragma once

#include <string>

namespace GameGuy {

	class Application {
	public:
		Application(const std::string& name) : mName(name) {}
		virtual ~Application() = default;

		virtual void onUpdate() = 0;
		virtual void onRender() = 0;
		virtual void onImGuiRender() = 0;

		const std::string& getName() const { return mName; }

	private:
		std::string mName;
	};

}