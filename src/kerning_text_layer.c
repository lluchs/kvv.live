// Copyright © 2015 Lukas Werling
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "kerning_text_layer.h"
struct KerningTextLayer {
	Layer *layer;
	GFont font;
	const char * text;
	const int * kerning;
};

static void draw_text(Layer *layer, GContext *ctx) {
	KerningTextLayer *text_layer = *(KerningTextLayer**)layer_get_data(layer);
	const char *c = text_layer->text;
	const int *k = text_layer->kerning;
	GRect box = layer_get_bounds(layer);
	box.origin = (GPoint) { .x = 0, .y = 0 };
	GTextOverflowMode overflow_mode = GTextOverflowModeFill;
	GTextAlignment alignment = GTextAlignmentLeft;

	graphics_context_set_text_color(ctx, GColorBlack);
	if (k == NULL) {
		graphics_draw_text(ctx, c, text_layer->font, box, overflow_mode, alignment, NULL);
	} else {
		char buf[2] = "\0";
		GSize size;
		for (; *c; c++, k++) {
			buf[0] = *c;
			box.origin.x += *k;
			box.size.w -= *k;
			size = graphics_text_layout_get_content_size(buf, text_layer->font, box, overflow_mode, alignment);
			if (box.size.w < size.w) break;
			graphics_draw_text(ctx, buf, text_layer->font, box, overflow_mode, alignment, NULL);
			box.origin.x += size.w;
			box.size.w -= size.w;
		}
	}
}

KerningTextLayer * kerning_text_layer_create(GRect frame) {
	KerningTextLayer *text_layer = malloc(sizeof(KerningTextLayer));
	*text_layer = (KerningTextLayer) {
		.layer = layer_create_with_data(frame, sizeof(KerningTextLayer*)),
		.font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
		.text = "",
		.kerning = NULL,
	};
	*(KerningTextLayer**)(layer_get_data(text_layer->layer)) = text_layer;
	layer_set_update_proc(text_layer->layer, draw_text);
	return text_layer;
}

void kerning_text_layer_destroy(KerningTextLayer * text_layer) {
	layer_destroy(text_layer->layer);
	free(text_layer);
}

Layer * kerning_text_layer_get_layer(KerningTextLayer * text_layer) {
	return text_layer->layer;
}

void kerning_text_layer_set_text(KerningTextLayer * text_layer, const char * text) {
	text_layer->text = text;
	layer_mark_dirty(text_layer->layer);
}

void kerning_text_layer_set_kerning(KerningTextLayer * text_layer, const int * kerning) {
	text_layer->kerning = kerning;
	layer_mark_dirty(text_layer->layer);
}

void kerning_text_layer_set_font(KerningTextLayer * text_layer, GFont font) {
	text_layer->font = font;
	layer_mark_dirty(text_layer->layer);
}
