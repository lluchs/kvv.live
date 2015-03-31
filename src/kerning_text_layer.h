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

#include <pebble.h>

typedef struct KerningTextLayer KerningTextLayer;

KerningTextLayer * kerning_text_layer_create(GRect frame);
void kerning_text_layer_destroy(KerningTextLayer * text_layer);
Layer * kerning_text_layer_get_layer(KerningTextLayer * text_layer);
void kerning_text_layer_set_text(KerningTextLayer * text_layer, const char * text);

// Sets kerning correction.
//
// `kerning` must be either NULL or an array of the same size as `text`.
void kerning_text_layer_set_kerning(KerningTextLayer * text_layer, const int * kerning);

void kerning_text_layer_set_font(KerningTextLayer * text_layer, GFont font);
// TODO: Other functions of TextLayer.
