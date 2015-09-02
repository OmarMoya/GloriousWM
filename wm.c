#include <xcb/xcb.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

xcb_connection_t *c;
xcb_screen_t *screen;
xcb_atom_t *atoms;
xcb_window_t root;
std::vector<std::string> atomNames;

// static const char *termcmd[] = { "/home/ludique/Desktop/MyWM/Examples/Events", NULL };
static const char *termcmd[] = { "gedit", NULL };
typedef union{
	const char** com;
	const int i;	
} Arg;

static void getAtoms();
static void setup(int);
static xcb_screen_t *screenOfDisplay(xcb_connection_t *, int);
static void spawnProgram(const char**);
int run();
unsigned int appCount;
int main()
{
	appCount = 0;
	bool myExit = false;
	int defaultScreen;
	atoms = new xcb_atom_t [4];
	c = xcb_connect(NULL, &defaultScreen );
	
	if (!c)
	{
		std::cerr << "connection failed!" << std::endl;
		return -1;	
	}

	atomNames.push_back("_NET_SUPPORTED");
	atomNames.push_back("_NET_WM_STATE_FULLSCREEN");
	atomNames.push_back("_NET_WM_STATE");
	atomNames.push_back("_NET_ACTIVE_WINDOW");								

	getAtoms();
	
	setup(defaultScreen);
	std::cout << "la zorra wmmm" << std::endl;

	xcb_window_t window;
	window = xcb_generate_id(c);
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2];
	values[0] = screen->white_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE;

//	xcb_create_window(c, 0, window, root,500, 0, 1500, 1500, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);
	//xcb_map_window(c, window);
	xcb_flush(c);
//	std::cout << "antes sapwn" << std::endl;
	spawnProgram(termcmd);
//	std::cout << "despues sapwn" << std::endl;
	xcb_generic_event_t *genEvent;
	while(!myExit)
	{
		if (genEvent = xcb_wait_for_event(c))
		{
			switch(genEvent->response_type & ~0x80)
			{
				case XCB_MAP_REQUEST:
					{
						appCount++;							
						xcb_map_request_event_t *e = (xcb_map_request_event_t *) genEvent;
				//		uint32_t values[] = {e->width, e->height};
					//	uint32_t values2[] = {appCount * 550};
				//		xcb_configure_window(c, e->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
						//xcb_configure_window(c, e->window, XCB_CONFIG_WINDOW_X, values2);
						
				//		uint32_t values2 = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
				//		xcb_change_window_attributes(c, e->window, XCB_CW_EVENT_MASK, &values2);
						
						xcb_map_window(c, e->window);
						xcb_flush(c);
					//	std::cout << "wena chooooro wind id: "<< e->window << std::endl;
					//	std::cout << "width: "<< e->width << std::endl;
					//	std::cout << "height: "<< e->height << std::endl;

						break;
					}
				case XCB_EXPOSE:
					{
					/*	xcb_expose_event_t *e = (xcb_expose_event_t *) genEvent;
						uint32_t values[] = {e->width, e->height};
						xcb_configure_window(c, e->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
						xcb_map_window(c, e->window);
						xcb_flush(c);*/
					}
				case XCB_KEY_RELEASE:
					{
						xcb_key_release_event_t *e = (xcb_key_release_event_t *)genEvent;
						switch(e->detail)
						{
							case 9:
								std::cout << "exit GloriousWM" << std::endl;
								myExit = true;
						}
						break;
					}
			}
		}
		delete genEvent;	
	}

	free(screen);
	free(atoms);
	xcb_disconnect(c);

	return 0;
}

void getAtoms()
{
	int count = atomNames.size();
	xcb_intern_atom_cookie_t atomCookie[count];
	xcb_intern_atom_reply_t * atomReply;

	for (unsigned int i = 0; i < count; i++)
	{
		atomCookie[i] = xcb_intern_atom(c, 0, atomNames[i].size(), atomNames[i].c_str());
	}

	for (unsigned int i = 0; i < count; i++)
	{
		atomReply = xcb_intern_atom_reply(c, atomCookie[i], NULL);
		if(!atomReply)
		{
			std::cerr << "error al cargar el atom " + i << std::endl; 
		} else {
			atoms[i] = atomReply->atom;
			free(atomReply);
		}
	}
}

void setup(int screen_)
{
	xcb_void_cookie_t attCookie;
	xcb_generic_error_t *error;
	uint32_t values = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_KEY_RELEASE;
	screen = screenOfDisplay(c, screen_);	
	if(!screen)
	{
		std::cerr << "no screen" << std::endl;
		return;
	}

	root = screen->root;
	
//	xcb_change_property(c, XCB_PROP_MODE_REPLACE, root, atoms[0], XCB_ATOM_ATOM, 32, 4, atoms);
	attCookie = xcb_change_window_attributes_checked(c, root, XCB_CW_EVENT_MASK, &values);
	error = xcb_request_check(c, attCookie);
	if (error)
	{
		std::cout << "error atributo root window" << std::endl;
		return;
	}
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, root, atoms[0], XCB_ATOM_ATOM, 32, 4, atoms);
	xcb_flush(c);
}

xcb_screen_t *screenOfDisplay(xcb_connection_t *con, int screen)
{
	xcb_screen_iterator_t iter;
	iter = xcb_setup_roots_iterator(xcb_get_setup(con));

	for(; iter.rem; --screen, xcb_screen_next(&iter))
	{
		if (screen == 0)
			return iter.data;
	}

	return NULL;
}

void spawnProgram(const char** arg)
{
	if(fork()) return;

	int er = execvp((char*)arg[0], (char**)arg);
	std::cout << "error al correr programa: " << er << std::endl;
}













