#include "window/window.hpp"
#include "renderer/renderer.hpp"

int main()
{
    Window& window = *new Window();
    Renderer& renderer = *new Renderer(window);

	window.title = "luceo";

	window.init();
    renderer.init();

	// Frame Loop
	while (window.is_running)
	{
        window.update();
        float& dt = window.dt;

		printf("Delta Time: %f \n", dt);

		renderer.update(dt);
	}

	renderer.end();

    delete &renderer;
    delete &window;

	return 1;
}