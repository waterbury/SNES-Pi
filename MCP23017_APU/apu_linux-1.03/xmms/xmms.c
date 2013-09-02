/* hwapu - SPC music playback tools for real snes apu
 * Copyright (C) 2004-2005  Raphael Assenat <raph@raphnet.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <glib.h>
#include <gtk/gtk.h>
#include "xmms/plugin.h"
#include "xmms/configfile.h"
#include "xmms/util.h"

#include "../apu.h"
#include "../apuplay.h"
#include "../apu_ppdev.h"

#include "../id666.h"


static APU_ops *apu_ops;
static int g_playing = 0;
static int g_length = 0;
static struct timeval g_playstart_tv;
static char *g_device=NULL;
static GtkWidget *device;

#define DEFAULT_LENGTH_SECONDS	150 // 2min 30

static void hwapu_init(void)
{
	ConfigFile *cfgfile;

	cfgfile = xmms_cfg_open_default_file();

	xmms_cfg_read_string(cfgfile, "HWAPU", "device", &g_device);
	if (!g_device) {
//		printf("Using default device\n");
		g_device = g_strdup("/dev/parport0");
	}
	else
	{
//		printf("Using device from config file: %s\n", g_device);
	}
	xmms_cfg_free(cfgfile);
}

static int hwapu_is_our_file(char *filename)
{
	FILE *fptr;
	unsigned char *magic = "SNES-SPC700 Sound File Data v0.30";
	unsigned char tmpbuf[34];

	fptr = fopen(filename, "rb");
	if (!fptr) { return 0; }

	tmpbuf[33] = 0;
	if (!fread(tmpbuf, 33, 1, fptr)) {
		fclose(fptr);
		perror("fread");
		return 0;
	}

	if (strcmp(tmpbuf, magic)==0) { 
		fclose(fptr);
//		printf("Is our file!\n");
		return 1; 
	}
	
	fclose(fptr);

	return 0;
}

static void hwapu_play_file(char *filename)
{
	FILE *fptr;
	id666_tag tag;
	int len;
	char *s;

	apu_ops = apu_ppdev_getOps();
	apu_setOps(apu_ops);
	if (apu_ops->init(g_device)<0) {
		fprintf(stderr, "failed to init ops\n");
		return;
	}
	
	fptr = fopen(filename, "rb");
	if (!fptr) { return; }
	
//	printf("Loading %s\n", filename);

	if (read_id666(fptr, &tag)) {
		len = strtol(
				(char*)tag.seconds_til_fadeout, 
				&s, 
				0);
		if ((unsigned char*)s == tag.seconds_til_fadeout) {
			g_length = DEFAULT_LENGTH_SECONDS * 1000;
		}
		else {
			g_length = len * 1000;
		}
	}
	
	if (LoadAPU(fptr)<0) {
		fprintf(stderr, "Problem\n");
		return;
	}

	gettimeofday(&g_playstart_tv, NULL);
	g_playing = 1;
	
	fclose(fptr);
}

static void hwapu_stop(void)
{
//	printf("Stop\n");
	apu_reset();
	g_playing = 0;
	apu_ops->shutdown();
}

static void hwapu_cleanup(void)
{
//	apu_ops->shutdown();
}

static int hwapu_get_time(void)
{
	struct timeval tv;
	int elaps_milli;

	gettimeofday(&tv, NULL);
	
	elaps_milli = (tv.tv_sec - g_playstart_tv.tv_sec) * 1000;
	elaps_milli += (tv.tv_usec/1000) - (g_playstart_tv.tv_usec/1000);
	
//	printf("Get time %d\n", elaps_milli);

 	if (elaps_milli > g_length) {
		return -1;
	}
	
	return elaps_milli;
}

static void hwapu_get_song_info(char *filename, char **title, int *length)
{
	FILE *fptr;
	char *s;
	int len;
	id666_tag tag;

	
	fptr = fopen(filename, "rb");
	if (!fptr) { return; }

	if (read_id666(fptr, &tag)) {
		*title = strdup(tag.title);
		len = strtol(
				(char*)tag.seconds_til_fadeout, 
				&s, 
				0);
		if ((unsigned char*)s == tag.seconds_til_fadeout) {
			*length = DEFAULT_LENGTH_SECONDS * 1000;
		}
		else {
			*length = len * 1000;
		}
	}
//	printf("get_song_info %s, %d\n", *title, *length);
	
	fclose(fptr);
}

static void hwapu_about()
{
	static GtkWidget *about_window=NULL;

	if (about_window)
		gdk_window_raise(about_window->window);

	about_window = xmms_show_message(
			"About hwapu SPC Plugin",
			"hwapu SPC player\n\n"
			"SPC loading code based on CaitSith2's Apuplay\n"
			"http://www.caitsith2.com/snes/apu.htm\n\n"
			"Linux port and xmms plugin by Raphael Assenat\n"
			"raph@raphnet.net\n\n"
			"Latest releases can be found here:\n"
			"http://www.raphnet.net/electronique/snes_apu/snes_apu.php",
			"Ok", FALSE, NULL, NULL);
	gtk_signal_connect(GTK_OBJECT(about_window), "destroy",
			GTK_SIGNAL_FUNC(gtk_widget_destroyed),
			&about_window);
			
}

/************* Configuration ************/
static GtkWidget *hwapu_configure_win = NULL;

static void hwapu_configurewin_ok_cb(GtkWidget *w, gpointer data)
{
	ConfigFile *cfgfile;

	if (g_device) {
		g_free(g_device);
	}

	g_device = g_strdup(gtk_entry_get_text(GTK_ENTRY(device)));
	
	cfgfile = xmms_cfg_open_default_file();

	xmms_cfg_write_string(cfgfile, "HWAPU", "device", g_device);
	xmms_cfg_write_default_file(cfgfile);
	xmms_cfg_free(cfgfile);
}

static void hwapu_configurewin_close(GtkButton *w, gpointer data)
{
	gtk_widget_destroy(hwapu_configure_win);
}

static void hwapu_configure(void)
{
	GtkWidget *vbox;
	//GtkWidget *dev_vbox;
	GtkWidget *dev_label;
	GtkWidget *dev_hbox;
	GtkWidget *bbox, *ok, *cancel;
	
	if (hwapu_configure_win) {
		return;
	}

	/* window */
	hwapu_configure_win = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_signal_connect(GTK_OBJECT(hwapu_configure_win), "destroy",
			GTK_SIGNAL_FUNC(gtk_widget_destroyed),
			&hwapu_configure_win);
	gtk_window_set_title(GTK_WINDOW(hwapu_configure_win),
			"hwapu SPC Player Configureation");
	gtk_window_set_policy(GTK_WINDOW(hwapu_configure_win),
			FALSE, FALSE, FALSE);
	gtk_window_set_position(GTK_WINDOW(hwapu_configure_win),
			GTK_WIN_POS_MOUSE);
	gtk_container_border_width(GTK_CONTAINER(hwapu_configure_win), 10);

	/* main vbox */
	vbox = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(hwapu_configure_win), vbox);
	

	/* hbox for device: field */
	dev_hbox = gtk_hbox_new(FALSE, 10);
	gtk_container_border_width(GTK_CONTAINER(dev_hbox), 0);
	gtk_box_pack_start(GTK_BOX(vbox), dev_hbox, FALSE, FALSE, 0);
		
	dev_label = gtk_label_new("parport device:");
	gtk_box_pack_start(GTK_BOX(dev_hbox), dev_label, FALSE, FALSE, 0);

	device = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(device), g_device);
	gtk_box_pack_start(GTK_BOX(dev_hbox), device, TRUE, TRUE, 0);
	
	
	/* ok cancel buttons */
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
	gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

	ok = gtk_button_new_with_label("Ok");
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			GTK_SIGNAL_FUNC(hwapu_configurewin_ok_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			GTK_SIGNAL_FUNC(hwapu_configurewin_close), NULL);
	GTK_WIDGET_SET_FLAGS(ok, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(bbox), ok, TRUE, TRUE, 0);
	gtk_widget_grab_default(ok);

	cancel = gtk_button_new_with_label("Cancel");
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			GTK_SIGNAL_FUNC(hwapu_configurewin_close), NULL);
	GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(bbox), cancel, TRUE, TRUE, 0);

	gtk_widget_show_all(hwapu_configure_win);
}


/****************** plugin *****************/
static InputPlugin hwapu_ip =
{
	NULL, // handle
	NULL, // filename
	"hwapu spc player", // description
	hwapu_init, // init
	hwapu_about, // about
	hwapu_configure, // configure,
	hwapu_is_our_file, // is_our_file
	NULL, // scan_dir
	hwapu_play_file, // play_file
	hwapu_stop, // stop
	NULL, // pause
	NULL, // seek
	NULL, // set_eq
	hwapu_get_time,
	NULL, // get_volume	
	NULL, // set_volume
	hwapu_cleanup, // cleanup
	NULL, // get_vis_type
	NULL, // ad_vis_pcm
	NULL, // set info
	NULL, // set_info_text
	hwapu_get_song_info, // get_song_info
	NULL, // file_info_box
	NULL // output plugin
};

InputPlugin *get_iplugin_info()
{
	return &hwapu_ip;
}

