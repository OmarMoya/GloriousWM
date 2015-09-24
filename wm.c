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
#include <FreeImage.h>
#include <xcb/xfixes.h>
#include <xcb/xcb_util.h>
xcb_window_t geditWindow;

xcb_connection_t *c;
xcb_screen_t *screen;
xcb_atom_t *atoms;
xcb_window_t root;
std::vector<std::string> atomNames;
unsigned int appCount;
	xcb_render_picture_t picture;
// static const char *termcmd[] = { "/home/ludique/Desktop/MyWM/Examples/Events", NULL };
static const char *termcmd[] = { "gedit", NULL };
typedef union{
	const char** com;
	const int i;	
} Arg;

xcb_render_picture_t bufferPicture;

static xcb_render_pictvisual_t * xcb_render_util_find_visual_format2 (const xcb_render_query_pict_formats_reply_t *formats, const xcb_visualid_t visual);

xcb_render_pictvisual_t *
xcb_render_util_find_visual_format2 (const xcb_render_query_pict_formats_reply_t *formats,
					       const xcb_visualid_t visual)
{
	    xcb_render_pictscreen_iterator_t screens;
		    xcb_render_pictdepth_iterator_t depths;
			    xcb_render_pictvisual_iterator_t visuals;
				    if (!formats)
							return 0;
					    for (screens = xcb_render_query_pict_formats_screens_iterator(formats); screens.rem; xcb_render_pictscreen_next(&screens))
								for (depths = xcb_render_pictscreen_depths_iterator(screens.data); depths.rem; xcb_render_pictdepth_next(&depths))
										    for (visuals = xcb_render_pictdepth_visuals_iterator(depths.data); visuals.rem; xcb_render_pictvisual_next(&visuals))
														if (visuals.data->visual == visual)
																	    return visuals.data;
						    return 0;
}

static void renderWindow(xcb_window_t window);
static void createMenu(uint16_t xPos, uint16_t yPos);
static void getAtoms();
static void setup(int);
static xcb_screen_t *screenOfDisplay(xcb_connection_t *, int);
static void spawnProgram(const char**);
static bool checkCompositeSystemSupport();
int run();
static bool checkIfWindowIsMenuOrica(xcb_connection_t *con, xcb_window_t win);
static void SetWindowPixmap(xcb_window_t*);
int main()
{
	FreeImage_Initialise();
	FIBITMAP *bitmap = FreeImage_Allocate(320, 240, 24);
	if (bitmap) {
		unsigned char *bits = FreeImage_GetBits(bitmap);
		    // bitmap successfully created!
		std::cout << "sizeof: " <<sizeof(bits)<<std::endl;
			for(int i=0;i<320;++i){
				for(int j=0;j<240;++j){
					 for (int c = 0; c < 3; c++) {
					 	if (c == 0) bits[i * 240 * 3 + j * 3 + c] = 255;
					 	else bits[i * 240 * 3 + j * 3 + c] = 0;
					// bits[c] = 255;
					// 
					 }
					// bits[0] = 255;
					// bits[1] = 0;
					// bits[2] = 0;

					// bits[3] = 0;
					// bits[4] = 255;
					// bits[5] = 0;
					// 
					// bits[6] = 0;
					// bits[7] = 0;
					// bits[8] = 255;
					// *(bits++)=j<255?j:255;
					// *(bits++)=i;
					// *bits=0;
				}
			}
			if (FreeImage_Save(FIF_BMP, bitmap, "mybitmap.bmp", 0)) {
				std::cout << "SALIO" << std::endl;
			 }
		     FreeImage_Unload(bitmap);
			}
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
	
/*	if(!checkCompositeSystemSupport())
	{
		delete screen;
		xcb_disconnect(c);
	}*/

	getAtoms();
	
	setup(defaultScreen);
	std::cout << "la zorra wmmm" << std::endl;

	if(!checkCompositeSystemSupport())
	{
		delete screen;
		xcb_disconnect(c);
	}

//	xcb_window_t window;
//	window = xcb_generate_id(c);
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
	bool done = false;
	while(!myExit)
	{
		if (genEvent = xcb_wait_for_event(c))
		{
			switch(genEvent->response_type & ~0x80)
			{
				case XCB_MAP_REQUEST:
					{
				//		std::cout << "Map request" << std::endl;
					//	appCount++;							
						xcb_map_request_event_t *e = (xcb_map_request_event_t *) genEvent;
					//	renderWindow(e->window);
					//	SetWindowPixmap(&e->window);	
						xcb_map_window(c, e->window);
						xcb_flush(c);
					//	SetWindowPixmap(&e->window);
					 	geditWindow = e->window;
						
						xcb_window_t overlyWin;
						xcb_composite_get_overlay_window_reply_t * overlayReply = xcb_composite_get_overlay_window_reply(c, xcb_composite_get_overlay_window(c, root), NULL);

						if(!overlayReply)
							return 1;
						std::cout << "overlay id " << overlayReply->overlay_win << " root id: " << root << std::endl;
					/*	xcb_get_window_attributes_cookie_t attCookie = xcb_get_window_attributes(c, e->window);	
						xcb_get_window_attributes_reply_t *attReply = xcb_get_window_attributes_reply(c, attCookie, NULL);
						if (!attReply)
						{
							std::cout << "error de atributos" << std::endl;
							break;
						}
						const uint32_t v = XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS;
					
						xcb_render_query_pict_formats_cookie_t cookieFormats = xcb_render_query_pict_formats(c);
						xcb_render_query_pict_formats_reply_t *replyFormats = xcb_render_query_pict_formats_reply (c, cookieFormats, NULL);
						
						xcb_render_pictvisual_t *pv = xcb_render_util_find_visual_format2( replyFormats, attReply->visual);
						
						std::cout << "pictVisual format " << pv->format << std::endl;
						
						xcb_render_picture_t rp = xcb_generate_id(c);
					    xcb_render_create_picture(c, rp, e->window, pv->format, XCB_RENDER_CP_REPEAT, &v);
						
						xcb_pixmap_t pixmap = xcb_generate_id(c);
						xcb_create_pixmap(c, screen->root_depth, pixmap, screen->root, screen->width_in_pixels, screen->height_in_pixels);

						xcb_render_picture_t destiny = xcb_generate_id(c);
						xcb_render_create_picture(c, destiny, pixmap, pv->format, 0, NULL);
							
						xcb_render_picture_t rootPic = xcb_generate_id(c);
						xcb_render_create_picture(c, rootPic, screen->root, pv->format, XCB_RENDER_CP_SUBWINDOW_MODE, &v);

						xcb_render_composite(c, XCB_RENDER_PICT_OP_SRC, destiny, XCB_NONE, rootPic,0,0, 0, 0, 0, 0, screen->width_in_pixels, screen->height_in_pixels);						

						xcb_free_pixmap(c, pixmap);

						xcb_render_composite(c, XCB_RENDER_PICT_OP_SRC, rp, XCB_NONE, destiny, 0, 0, 0, 0, 0, 0, 500, 500);
						std::cout << "map request" << std::endl;
						xcb_flush(c);*/
						break;
					}
				case XCB_COLORMAP_NOTIFY:
					{
						xcb_colormap_notify_event_t *e = (xcb_colormap_notify_event_t*) genEvent;
						SetWindowPixmap(&e->window);
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
							if (!checkIfWindowIsMenuOrica(c, e->event))
							{
								std::cout << "Button Press" << std::endl;
							//	xcb_unmap_window(c, geditWindow);
								SetWindowPixmap(&geditWindow);
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

void renderWindow(xcb_window_t window)
{
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2];
	values[0] = screen->white_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE;
	
	
	//Pedimos los atributos de la ventana
	xcb_get_window_attributes_cookie_t windowAtrributeCookie;
	xcb_get_window_attributes_reply_t *windowAttributeReply;

	windowAtrributeCookie = xcb_get_window_attributes(c, window);
	windowAttributeReply = xcb_get_window_attributes_reply(c, windowAtrributeCookie, NULL);

	if(!windowAttributeReply)
	{
		std::cout << "error carga de atributos" << std::endl;
	}
	
	//obtenemos el formato (revisar por errores)
	xcb_render_query_pict_formats_cookie_t windowFormatsCookie = xcb_render_query_pict_formats(c);
	xcb_render_query_pict_formats_reply_t *windowFormatsReply = xcb_render_query_pict_formats_reply(c, windowFormatsCookie, NULL);

	xcb_render_pictvisual_t *windowPictVisual = xcb_render_util_find_visual_format2(windowFormatsReply, windowAttributeReply->visual);

	//creamos un render picture
	const uint32_t value = XCB_SUBWINDOW_MODE_CLIP_BY_CHILDREN;
	picture = xcb_generate_id(c);
	xcb_render_create_picture(c, picture, window, windowPictVisual->format, XCB_RENDER_CP_SUBWINDOW_MODE, &value);

//	create our destiny window
	xcb_window_t myWindow = xcb_generate_id(c);
    xcb_create_window(c, 0, myWindow, root,0, 0, 1500, 1500, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

	//hacer render_picture donde se mostrara la ventana, y luego hacer el render_composit
	xcb_pixmap_t pixmap = xcb_generate_id(c);
	xcb_create_pixmap(c, screen->root_depth, pixmap, myWindow, screen->width_in_pixels, screen->height_in_pixels);
	
	xcb_render_query_pict_formats_reply_t *destinyFormats = xcb_render_query_pict_formats_reply(c, xcb_render_query_pict_formats(c), NULL);
	xcb_render_pictvisual_t *destinyPictVisual = xcb_render_util_find_visual_format2(destinyFormats, screen->root_visual);

	bufferPicture = xcb_generate_id(c);
	xcb_render_create_picture(c, bufferPicture, myWindow, destinyPictVisual->format, 0, NULL);


	//Shape handle
	xcb_xfixes_region_t wRegion = xcb_generate_id(c);
	xcb_xfixes_create_region_from_window(c, wRegion, window, XCB_SHAPE_SK_BOUNDING);
  
	xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(c, xcb_get_geometry(c, window), NULL);
 
	xcb_xfixes_translate_region(c, wRegion, geometry->x, geometry->y);	
	xcb_xfixes_set_picture_clip_region(c, picture, wRegion, 0, 0);
	xcb_xfixes_destroy_region(c, wRegion);

	xcb_render_composite(c,XCB_RENDER_PICT_OP_OVER, picture, XCB_NONE, myWindow, 0, 0, 0, 0,100, 100, 500, 500 );
//	std::cout << "asdada" << std::endl/
	xcb_composite_redirect_window(c, window, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
		xcb_map_window(c, window);
}

void SetWindowPixmap(xcb_window_t* window)
{
//	xcb_map_window(c, *window);
//	xcb_flush(c);
//	xcb_aux_sync(c);
	xcb_pixmap_t p = xcb_generate_id(c);
	
	xcb_composite_name_window_pixmap(c, *window, p);

	xcb_unmap_window(c, *window);
	xcb_get_image_reply_t* img = xcb_get_image_reply(c, xcb_get_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, p, 0, 0, 1300, 600, ~0), NULL);
	
	if(!img)
	{
		std::cout << "img error" << std::endl;
		return;
	}
	uint8_t *data = xcb_get_image_data(img);
	size_t l = xcb_get_image_data_length(img);
	FIBITMAP *bitmap = FreeImage_Allocate(1300, 600, 32);
	if (bitmap)
	{
		unsigned char *bits = FreeImage_GetBits(bitmap);
		for(unsigned int i = 0; i < 600; i++ )
		{
			for(unsigned int j = 0; j < 1300; j++)
			{
				for (unsigned int c = 0; c < 4; c++)
				{
					*(bits++) = *(data++);
				}
			}
		}
	}
	if (FreeImage_Save(FIF_BMP, bitmap, "mybitmap2.bmp", 0) )
	{
		                  std::cout << "SALIO" << std::endl;
	}
    FreeImage_Unload(bitmap);
	               

	free(img);
//	xcb_flush(c);
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
	xcb_composite_redirect_subwindows(c, screen->root, XCB_COMPOSITE_REDIRECT_MANUAL);
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








