/* 
   A simple MPD tray icon using GTK.
*/

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <math.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <mpd/song.h>
#include <mpd/client.h>
#include <mpd/status.h>

#include "icons.h"
#include "cmdline.h"

const guint8* icons[101] = {i0,i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20,i21,i22,i23,i24,i25,i26,i27,i28,i29,i30,i31,i32,i33,i34,i35,i36,i37,i38,i39,i40,i41,i42,i43,i44,i45,i46,i47,i48,i49,i50,i51,i52,i53,i54,i55,i56,i57,i58,i59,i60,i61,i62,i63,i64,i65,i66,i67,i68,i69,i70,i71,i72,i73,i74,i75,i76,i77,i78,i79,i80,i81,i82,i83,i84,i85,i86,i87,i88,i89,i90,i91,i92,i93,i94,i95,i96,i97,i98,i99,i100};

gboolean update_status_icon(gpointer conn);
gchar *get_song_info(struct mpd_song* song);

double round(double r)
{
    return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

/* Passing both the mpd connection and status icon between
   callbacks would require an illogical and confusing data structure
   so I define the status icon globally for clarity. */
GtkStatusIcon* status_icon;

int main(int argc, char** argv)
{
    /* Parse command line arguments using gengetopt
       Default values are defined in ggo file */
    struct gengetopt_args_info args_info;
    cmdline_parser(argc, argv, &args_info);

    const unsigned int port = args_info.port_arg;        // Default: 6600
    const char* host = strdup(args_info.host_arg);       // Default: localhost
    const unsigned int timeout = args_info.timeout_arg;  // Default: 0 (libmpdclient default)

    cmdline_parser_free(&args_info);

    // Connect to MPD
    struct mpd_connection* conn = mpd_connection_new(host, port, timeout);

    if(mpd_connection_get_error(conn))
    {
        printf("Error connecting to mpd server at %s:%d. %s.\n",
                host, port, mpd_connection_get_error_message(conn));
        mpd_connection_free(conn);
        return 1;
    }

    // Initialize GTK
    if(!gtk_init_check(&argc, &argv))
    {
        printf("Failed to initialize gtk.\n");
        return 1;
    }

    status_icon = gtk_status_icon_new();

    // update the status icon immediately
    update_status_icon((gpointer)conn);
    // 
    g_timeout_add_seconds(1, (GSourceFunc)update_status_icon, (gpointer)conn);

    gtk_main();

    mpd_connection_free(conn);
    return 0;
}

float get_song_fraction_played(struct mpd_status* status)
{
    return (float)mpd_status_get_elapsed_time(status)/mpd_status_get_total_time(status);
}

gboolean update_status_icon(gpointer conn)
{
    struct mpd_song* song = mpd_run_current_song(conn);

    if (song == NULL)
	gtk_status_icon_set_has_tooltip(status_icon, FALSE);
    else
        gtk_status_icon_set_tooltip_markup(status_icon, (gchar*)get_song_info(song));

    struct mpd_status* status = mpd_run_status(conn);
    enum mpd_state state = mpd_status_get_state(status);

    if (state == MPD_STATE_PLAY)
    {
        unsigned int i = round(100*get_song_fraction_played(status));
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_inline (-1, icons[i], FALSE, NULL);
	gtk_status_icon_set_from_pixbuf(status_icon, pixbuf);
    }
    else if(state == MPD_STATE_PAUSE)
        gtk_status_icon_set_from_icon_name(status_icon, "media-playback-pause");
    else if(state == MPD_STATE_STOP)
    {
	// libmpdclient bug means a current song is reported even if stopped, so unset tooltip here
	gtk_status_icon_set_has_tooltip(status_icon, FALSE);
        gtk_status_icon_set_from_icon_name (status_icon, "media-playback-stop");
    }
    else
    {
        printf("Unrecognized mpd state: %d\n", state);
        return FALSE;
    }

    return TRUE;
}

const int title_type_count = 3;
const enum mpd_tag_type title_types[] = {
    MPD_TAG_TITLE,
    MPD_TAG_TRACK,
    MPD_TAG_NAME,
};

const int artist_type_count = 4;
const enum mpd_tag_type artist_types[] = {
    MPD_TAG_ARTIST,
    MPD_TAG_ALBUM_ARTIST,
    MPD_TAG_COMPOSER,
    MPD_TAG_PERFORMER,
};

/* borrowed from yshui/tprj */
gchar* get_song_info(struct mpd_song* song)
{
    //Title
    const gchar* title = NULL;
    int i;
    for(i = 0; !title && i<title_type_count; ++i)
    {
        title = mpd_song_get_tag(song, title_types[i], 0);
        title = g_markup_escape_text(title, -1);
    }

    //Artist
    const gchar* artist = NULL;
    for(i = 0; !artist && i<artist_type_count; ++i)
    {
        artist = mpd_song_get_tag(song, artist_types[i], 0);
        artist = g_markup_escape_text(artist, -1);
    }

    //Album
    const gchar* album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
    if(!album) album = mpd_song_get_tag(song, MPD_TAG_DISC, 0);
    album = g_markup_escape_text(album, -1);

    gchar *song_name;
    if(album || artist || title)
        song_name = g_strdup_printf("<b>%s</b> from <b>%s</b> by <b>%s</b>", title?title:"", album?album:"", artist?artist:"");
    else
    {
        const gchar *uri = mpd_song_get_uri(song);
        gchar *tmp_uri = g_strdup(uri);
        const gchar *file_name = basename(tmp_uri);
        g_free(tmp_uri);
        song_name = g_strdup(file_name);
    }
    return song_name;
}
