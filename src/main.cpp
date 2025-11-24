#include <cstdio>

#include "window/window.hpp"

int main()
{
    Window& window = *new Window();

	window.title = "luceo";

	window.init();

	// Frame Loop
	while (window.is_running)
	{
        window.update();
	}

    delete &window;

	return 1;
}