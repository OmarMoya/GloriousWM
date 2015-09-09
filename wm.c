#include <xcb/xcb.h>
#include <cstring> //memcpy
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <xcb/composite.h>
#include <xcb/render.h>
#include <xcb/xcb_renderutil.h>

xcb_connection_t *c;
xcb_screen_t *screen;
xcb_atom_t *atoms;
xcb_window_t root;
std::vector<std::string> atomNames;
unsigned int appCount;
// static const char *termcmd[] = { "/home/ludique/Desktop/MyWM/Examples/Events", NULL };
static const char *termcmd[] = { "gedit", NULL };
typedef union{
	const char** com;
	const int i;	
} Arg;

static void createMenu(uint16_t xPos, uint16_t yPos);
static void getAtoms();
static void setup(int);
static xcb_screen_t *screenOfDisplay(xcb_connection_t *, int);
static void spawnProgram(const char**);
static bool checkCompositeSystemSupport();
int run();
static bool checkIfWindowIsMenuOrica(xcb_connection_t *con, xcb_window_t win);
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
	
	checkCompositeSystemSupport();

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
//	xcb_flush(c);
//	std::cout << "antes sapwn" << std::endl;
	spawnProgram(termcmd);
//	createMenu();
//	callMenu(10, 0);
//	std::cout << "despues sapwn" << std::endl;
	xcb_generic_event_t *genEvent;
	while(!myExit)
	{
		if (genEvent = xcb_poll_for_event(c))
		{
			switch(genEvent->response_type & ~0x80)
			{
				case XCB_MAP_REQUEST:
					{
					//	std::cout << "Map request" << std::endl;
					//	appCount++;							
						xcb_map_request_event_t *e = (xcb_map_request_event_t *) genEvent;
				
					//	xcb_map_window(c, e->window);
					//	xcb_flush(c);
						
						const uint32_t v = XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS;
					
						xcb_render_query_pict_formats_cookie_t cookieFormats = xcb_render_query_pict_formats(c);
						xcb_render_query_pict_formats_reply_t *replyFormats = xcb_render_query_pict_formats_reply (c, cookieFormats, NULL);
						
						xcb_render_pictvisual_t *pv = xcb_render_util_find_visual_format( replyFormats, screen->root_visual);
						xcb_render_picture_t rp = xcb_generate_id(c);
						xcb_void_cookie_t cookie = xcb_render_create_picture_checked(c, rp, (xcb_drawable_t) e->window, pv->format, XCB_RENDER_CP_SUBWINDOW_MODE, &v);
						break;
					}
				case XCB_EXPOSE:
					{
						//std::cout << "Expose" << std::endl;
						/*xcb_expose_event_t *e = (xcb_expose_event_t *) genEvent;
						xcb_map_window(c, e->window);
						xcb_flush(c);*/
						break;
					}
				case XCB_BUTTON_PRESS:
					{
						xcb_button_press_event_t *e = (xcb_button_press_event_t *) genEvent;
						if(e->detail == 3)
						{
							if (checkIfWindowIsMenuOrica(c, e->event))
							{
								break;
							}
							createMenu(e->event_x, e->event_y);
						} else if (e->detail == 1)
						{
							if (checkIfWindowIsMenuOrica(c, e->event))
							{
								xcb_destroy_window(c, e->event);
								xcb_flush(c);
								break;
							}
						/*	if (e->event == wMenu)
							{
								break;
							}
							std::cout << "root " << root  << std::endl;
							std::cout << "menu " << wMenu  << std::endl;
							std::cout << "window of event " << e->event  << std::endl;
							xcb_unmap_window(c, wMenu);
							xcb_flush(c);
							break;*/
						}
						break;
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
bool checkCompositeSystemSupport()
{  
   	xcb_generic_error_t *error;	
	xcb_composite_query_version_cookie_t cookie;
	xcb_composite_query_version_reply_t *reply;
	uint32_t minorVersion = 0, majorVersion = 2;

	cookie = xcb_composite_query_version(c, minorVersion, majorVersion);

	reply = xcb_composite_query_version_reply(c, cookie, &error);
	
	if(error)
	{		
		std::cout << "error" << error->error_code  << std::endl;
	}
	
	if (!reply)
	{
		std::cout << "no se pudo obtener version composite" << std::endl;
		return false;
	}
	//si es compatible, redireccionamos las ventanas a la memoria offscreen
	xcb_composite_redirect_subwindows(c, root, 0);
	std::cout << "min " <<  reply->minor_version << "max: " << reply->major_version <<  std::endl;
	return true;
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
	if (c) close(screen->root);
	int er = execvp((char*)arg[0], (char**)arg);
	std::cout << "error al correr programa: " << " error: "  << er << std::endl;
//	exit(EXIT_SUCCES);
}

void createMenu( uint16_t xPos, uint16_t yPos )
{
 	xcb_window_t wMenu = xcb_generate_id(c);
	std::string windowTitle = "Menu Orica"; 
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2];
	int numberOfButtons = 3;
	std::string buttonText[numberOfButtons];
	values[0] = screen->white_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS;

	xcb_create_window(c, 0, wMenu, root,0, 0, 125, 140, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

	xcb_change_property(c, XCB_PROP_MODE_REPLACE, wMenu, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, windowTitle.size(), windowTitle.c_str());

	mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y; 
	values[0] = xPos;
	values[1] = yPos;
	
	xcb_configure_window(c, wMenu, mask, values);
	xcb_map_window(c, wMenu);

	for (unsigned int i = 0; i < numberOfButtons; i++)
	{
		xcb_window_t w = xcb_generate_id(c);

		mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
		values[0] = screen->black_pixel;
		values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS;

		xcb_create_window(c, 0, w, wMenu, 2, (40 * i) + 5, 120, 30, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);
		


		xcb_map_window(c, w);
	}

	xcb_flush(c);
}

bool checkIfWindowIsMenuOrica(xcb_connection_t *con, xcb_window_t win)
{
	bool isMenuOrica = false;
	int len;
	char *windowName;
	xcb_get_property_cookie_t cookie;
	xcb_get_property_reply_t *reply;

	cookie = xcb_get_property(con, 0, win, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 4);
	reply = xcb_get_property_reply(con, cookie, NULL);
	if (!reply)
	{
		std::cout << "Error Menu Orica" << std::endl;
		delete reply;
		return false;
	}
	
	len = xcb_get_property_value_length(reply);
	windowName = new char[len + 1];
	memcpy(windowName, xcb_get_property_value(reply), len);
	windowName[len] = '\0';
//	std::cout<< "prop: " << windowName << "length: " << len <<std::endl;
	
	if (!strcmp(windowName, "Menu Orica")) //retorna 0 si son iguales
	{
		isMenuOrica = true;
	} else {
		isMenuOrica = false;
	}
	
	delete reply;
	delete[] windowName;
	return isMenuOrica;
}








