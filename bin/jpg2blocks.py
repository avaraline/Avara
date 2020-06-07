#!/usr/bin/env python3
# takes a png image and makes an avara block sculpture svg that matches
import math
from pict2svg import SVGContext, Rect, Color, arc_path
from lxml import etree
from PIL import Image

white = Color(65535, 65535, 65535)
black = Color(0, 0, 0)

block_size = 10

def scale_color(c):
	return math.floor(c * 65535 / 255)

def color_from_pixel(r, g, b):
	return Color(scale_color(r), scale_color(g), scale_color(b))


def make_rect(context, color, x, y, w, h):
	rect = Rect(y, x, y + h, x + w);
	svgrect = context.element("rect")
	context.rect_data(rect, svgrect)
	context.fg = color
	context.fill(svgrect)
	context.fg = Color(0, 0, 0)
	context.stroke(svgrect)

def make_arc(context, fill, stroke, x, y, start, end):
	rect = Rect(y, x, y + 30, x + 30)
	arc = context.element("path")
	arc_path(context, rect, start, end, arc)
	context.fg = stroke
	context.stroke(arc)
	context.fg = fill
	context.fill(arc)
	return arc

def level_base(context):
	make_arc(context, white, black, 50, -20, 337, 45)
	context.text("object Incarnator\ry = 10\rend", x=50, y=-20)
	context.flush_text()

	make_arc(context, white, black, -100, -100, 0, 180)
	context.textpos.x = -100
	context.textpos.y = -100
	context.text("adjust GroundColor\rend")
	context.flush_text()
	sky = make_arc(context, black, white, -100, 0, 0, 180)
	context.textpos.y = 0
	context.text("adjust SkyColor\rend")
	context.flush_text()



if __name__ == "__main__":
	s = SVGContext(Rect(0, 0, 1000, 1000))
	# add an incarn and b/w sky/ground for contrast
	level_base(s)

	# open image
	im = Image.open("bin/windsurfer.jpg")
	# this is the output resolution in blocks
	# 32x32 takes like 10 seconds to load
	# so you want to keep this as small as possible
	im.thumbnail((16,16))

	for x in range(0, im.size[0]):
		for y in range(0, im.size[1]):
			block_pos_x = x * block_size
			block_pos_y = y * block_size
			block_dim = block_size + 1
			r, g, b = im.getpixel((x, y))
			color = color_from_pixel(r, g, b)
			make_rect(s, color, block_pos_x, block_pos_y, block_dim, block_dim)

	with open("test.svg", "w") as xml_file:
		svg_string = s.close("")
		xml_file.write(svg_string)



