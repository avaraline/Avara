from decimal import *
from .datatypes import *
from lxml import etree as ET
import shlex


def heading_from_arc(startAngle, angle):
        heading = (startAngle + (angle / 2)) + 180

        while(heading >= 360):
            heading -= 360

        return heading


class Converter:
    SCALE = Decimal(18)
    SNAP = Decimal('.001')

    def __init__(self):

        self.name = None           # Stores the map name
        self.tagline = None        # Map tagline
        self.author = None         # Map author
        self.description = None    # Map description
        self.comments = []

        self.replace_vars = {}     # Dict of replacement variable values
        self.goodies = []          # List of all goodies
        self.incarnators = []      # List of all incarnators
        self.teleporters = []      # List of all teleporters
        self.blocks = []           # List of all static blocks
        self.ramps = []            # List of all static ramps
        self.free_solids = []      # List of all free solids
        self.g_vars = []           # List of globally concerned objects
        self.enums = {}            # Dict of all enums
        self.lights = []
        self.ambient = 0.4

        self.wall_height = 3       # Current wall height (default 3)
        self.wa = 0                # Current wa, resets after every wall
        self.last_wa = 0           # Temporary wa for ramps
        self.base_height = 0       # Current base height
        self.pixel_to_thickness = Decimal("0.125")   # current corner-radius to thickness factor

        self.fg_color = Color()    # Last foreground colour
        self.cur_block = None      # Last created block
        self.cur_arc = None        # Last arc
        self.cur_text = ""         # Placeholder for text stack
        self.last_rect = None      # Last Rect used on a block
        self.last_arc = None       # Last Rect used for an arc

        self.origin_x = 0          # Current origin x
        self.origin_y = 0          # Current origin y

        self.pen_x = 1             # Current pen x
        self.pen_y = 1             # Current pen y

        # When the origin is changed some PICT calls
        # mean different things, thus why we store
        # data on different 'types' of origin change
        self.block_origin_changed = False
        self.arc_origin_changed = False

    def get_wall_points_from_rect(self, rect, corner_radius=0):
        size = Point3D()
        center = Point3D()

        # Assuming here that source is always top left
        # and dest is always bottom right seems to be
        # true based on all PICTs checked
        #
        # Also assuming that the 'size' is the full
        # length/width/height, not halved
        #
        # PICT y becomes x, PICT x becomes z, and height
        # becomes y. 

        real_src_x = (rect.src.x + self.origin_x)
        real_src_y = (rect.src.y + self.origin_y)
        real_dst_x = (rect.dst.x + self.origin_x)
        real_dst_y = (rect.dst.y + self.origin_y)

        # Pen size does actually affect the size of the block
        size.z = Decimal(real_dst_x - real_src_x) - self.pen_x
        size.x = Decimal(real_dst_y - real_src_y) - self.pen_y
        if corner_radius is not 0:
            size.y = Decimal(str(corner_radius * self.pixel_to_thickness))
        else:
            size.y = Decimal(self.wall_height)
        center.z = Decimal(real_src_x + real_dst_x) / Decimal(2)
        center.x = Decimal(real_src_y + real_dst_y) / Decimal(2)
        if corner_radius is not 0:
            center.y = (Decimal(str(corner_radius * self.pixel_to_thickness)) / Decimal(2)) + self.wa + self.base_height
        else:
            center.y = (Decimal(self.wall_height) / Decimal(2)) + self.wa + self.base_height

        # No need to scale y because y is only ever
        # indicated in meters (except where rounded rects come into play)
        size.x = self.scale_and_snap(size.x)
        size.z = self.scale_and_snap(size.z)
        center.x = self.scale_and_snap(center.x)
        center.z = self.scale_and_snap(center.z)

        # Reset wa because it's a per wall variable
        self.last_wa = self.wa
        self.wa = 0

        return size, center

    def convert(self, ops):
        getcontext().prec = 10
        lastText = False
        thisText = False
        commentText = False
        if ops is None:
            return
        for op in ops:
            classname = op.__class__.__name__
            if classname in ("LongText",
                             "DHText",
                             "DVText",
                             "DHDVText"):
                thisText = True
            elif lastText and classname in ("TextFont",
                                            "TextSize",
                                            "VariableReserved"):
                thisText = True
            else:
                thisText = False
                if lastText:
                    # if "=" is at the end, we can't
                    # pass to shlex until we have a bit more
                    if not self.cur_text[-1] == "=": 
                        if "/*" in self.cur_text:
                            # start parsing comments
                            commentText = True
                        if "*/" in self.cur_text:
                            commentText = False
                            self.comments.append(self.cur_text)
                            # we don't even want to wparse comments
                            self.cur_text = ""
                        if not commentText and self.cur_text:
                            self.parse_text(self.cur_text)
                        self.cur_text = ""

            # TODO: PARSE OVALS!
            if classname == "ClipRegion":
                self.cur_region = op.region

            # A positive dv/dh moves you backwards
            # This is confusing.
            elif classname == "Origin":
                self.origin_x -= op.dv
                self.origin_y -= op.dh
                self.block_origin_changed = True
                self.arc_origin_changed = True
                self.cur_block = None
            elif classname == "PenSize":
                self.pen_x = op.size.x
                self.pen_y = op.size.y
            elif classname == "RGBForegroundColor":
                self.fg_color = Color.from_rgb(op.red, op.green, op.blue)
            elif classname == "RGBBackgroundColor":
                self.fg_color = Color.from_rgb(op.red, op.green, op.blue)
            elif classname == "FrameRectangle":
                if self.block_origin_changed:
                    self.block_origin_changed = False
                block = self.create_block(op.rect)
                self.cur_block = block
                self.blocks.append(block)

            # Avara does not create a wall until framed so here
            # we're just storing the rectangle and color
            # This also means that we can be sure that the
            # right pen size is used at framing time
            elif classname == "PaintRectangle":
                if self.block_origin_changed:
                    self.block_origin_changed = False
                self.last_rect = op.rect
                self.block_color = self.fg_color

            # A note for Frame/Paint Same Rectangle:
            # If the origin point changes and then one
            # of these is called, it means use the same
            # rectangle dimensions from the new origin point
            # but actually create a new rectangle
            elif classname == "FrameSameRectangle":
                self.cur_block = self.create_block(self.last_rect)

                if self.block_origin_changed:
                    self.block_origin_changed = False
                else:  # Block has been painted
                    self.cur_block.color = self.block_color

                self.blocks.append(self.cur_block)
            elif classname == "PaintSameRectangle":
                if self.block_origin_changed:
                    self.block_origin_changed = False
                    self.block_color = self.fg_color
                else:
                    self.cur_block.color = self.fg_color

            elif classname == "OvalSize":
                self.curr_oval_size = op.size

            elif classname == "FrameRoundedRectangle":
                if self.block_origin_changed:
                    self.block_origin_changed = False
                block = self.create_block(
                    op.rect,
                    corner_radius=self.curr_oval_size.x)
                self.cur_block = block
                if self.curr_oval_size:
                    self.cur_block.rounding = self.curr_oval_size
                self.blocks.append(block)
            elif classname == "PaintRoundedRectangle":
                if self.block_origin_changed:
                    self.block_origin_changed = False
                self.last_rect = op.rect
                self.block_color = self.fg_color

            elif classname == "FrameSameRoundedRectangle":
                self.cur_block = self.create_block(
                    self.last_rect,
                    corner_radius=self.curr_oval_size.x)

                if self.block_origin_changed:
                    self.block_origin_changed = False
                else:  # Block has been painted
                    self.cur_block.color = self.block_color

                self.blocks.append(self.cur_block)
            elif classname == "PaintSameRoundedRectangle":
                if self.block_origin_changed:
                    self.block_origin_changed = False
                    self.block_color = self.fg_color
                else:
                    self.cur_block.color = self.fg_color

            elif classname == "FrameArc":
                if self.arc_origin_changed:
                    self.arc_origin_changed = False
                self.cur_arc = self.create_arc(
                    op.rect, op.startAngle, op.arcAngle)
                self.cur_arc.stroke = self.fg_color
            elif classname == "PaintArc":
                if self.arc_origin_changed:
                    self.arc_origin_changed = False
                self.cur_arc = self.create_arc(
                    op.rect, op.startAngle, op.arcAngle)
                self.cur_arc.fill = self.fg_color

            # The same Arc functions can result in different
            # arcs if the angles are different
            elif classname == "FrameSameArc":
                if self.arc_origin_changed:
                    self.arc_origin_changed = False
                    self.cur_arc = self.create_arc(
                        self.last_arc, op.startAngle, op.arcAngle)
                else:
                    heading = heading_from_arc(op.startAngle, op.arcAngle)
                    if self.cur_arc.heading != heading:
                        self.cur_arc = self.create_arc(
                            self.last_arc, op.startAngle, op.arcAngle)

                self.cur_arc.stroke = self.fg_color

            elif classname == "PaintSameArc":
                if self.arc_origin_changed:
                    self.arc_origin_changed = False
                    self.cur_arc = self.create_arc(
                        self.last_arc, op.startAngle, op.arcAngle)
                else:
                    heading = heading_from_arc(op.startAngle, op.arcAngle)
                    if self.cur_arc.heading is not heading:
                        self.cur_arc = self.create_arc(
                            self.last_arc, op.startAngle, op.arcAngle)

                self.cur_arc.fill = self.fg_color

            elif thisText:
                if hasattr(op, 'text'):
                    if lastText:
                        self.cur_text += " " + op.text
                    else:
                        self.cur_text = op.text

            if thisText:
                lastText = True
            else:
                lastText = False

        mapEl = ET.Element('map')

        for g_var in self.g_vars:
            g_var.to_xml(mapEl)

        for inc in self.incarnators:
            inc.to_xml(mapEl)

        staticEl = ET.SubElement(mapEl, 'static')
        for static in self.blocks:
            static.to_xml(staticEl)

        for static in self.ramps:
            static.to_xml(staticEl)

        for good in self.goodies:
            good.to_xml(mapEl)

        for fs in self.free_solids:
            fs.to_xml(mapEl)

        if self.author:
            mapEl.set('designer', self.author)
        if self.description:
            mapEl.set('description', self.description)

        # ET.dump(mapEl)
        return mapEl

    def create_block(self, rect, corner_radius=0):
        self.last_rect = rect
        block = Block()
        size, center = self.get_wall_points_from_rect(
            rect, corner_radius=corner_radius)
        block.size = size
        block.center = center
        return block

    def create_arc(self, rect, startAngle, arcAngle):
        arc = Arc()
        center = Point3D()
        self.last_arc = rect

        real_src_x = rect.src.x + self.origin_x
        real_src_y = rect.src.y + self.origin_y
        real_dst_x = rect.dst.x + self.origin_x
        real_dst_y = rect.dst.y + self.origin_y

        # Flipping x and z around to better match pavara
        center.z = Decimal(real_src_x + real_dst_x) / Decimal(2)
        center.x = Decimal(real_src_y + real_dst_y) / Decimal(2)
        center.y = self.base_height

        center.x = self.scale_and_snap(center.x)
        center.z = self.scale_and_snap(center.z)

        arc.center = center
        arc.heading = heading_from_arc(startAngle, arcAngle)

        return arc

    def parse_text(self, text):
        in_unique = False
        in_object = False
        in_adjust = False
        in_enum = False
        cur_object = {}
        cur_enum = None

        try:
            words = shlex.split(text)
        except ValueError:
            text += '"'
            words = shlex.split(text)

        newwords = []

        skipnext = False
        for idx, word in enumerate(words):
            if '=' in word:
                if len(word) == 1 and idx + 1 not in words:
                    continue
                if len(word) == 1:
                    newwords.pop()
                    newwords.append((words[idx - 1], words[idx + 1]))
                    skipnext = True
                elif word.endswith('='):
                    newwords.append((word[:-1], words[idx + 1]))
                    skipnext = True
                elif word.startswith('='):
                    newwords.pop()
                    newwords.append((words[idx - 1], word[1:]))
                else:
                    splitword = word.split('=')
                    newwords.append((splitword[0], splitword[1]))
            elif skipnext:
                skipnext = False
            else:
                newwords.append(word)

        for word in newwords:
            if word == "unique":
                in_unique = True
            elif word == "object":
                in_object = True
            elif word == "adjust":
                in_adjust = True
            elif word == "enum":
                in_enum = True
            elif word == "end":
                if in_unique:
                    in_unique = False
                elif in_object:
                    in_object = False
                    self.parse_object(cur_object)
                    cur_object = {}
                elif in_adjust:
                    in_adjust = False
                elif in_enum:
                    in_enum = False
                    cur_enum = None
            elif in_object:
                if len(cur_object) == 0:
                    cur_object['type'] = word
                elif type(word) is tuple:
                    cur_object[word[0]] = word[1]
            elif in_unique:
                # Do nothing
                pass
            elif in_adjust:
                self.parse_adjust(word)
            elif in_enum:
                if cur_enum is None:
                    cur_enum = Decimal(word)
                else:
                    self.enums[word] = cur_enum
                    cur_enum += 1
            elif type(word) is tuple:
                self.parse_global_variable(word[0], word[1])

    def parse_global_variable(self, key, value):
        if key[:5] == 'light':
            idx = int(key.split("[")[1][0])
            if idx not in self.lights:
                self.lights.append(Light())
            op = key.split("]")[1][1]
            if op == "i":
                self.lights[idx].intensity = value
            if op == "a":
                self.lights[idx].elevation = value
            if op == "b":
                self.lights[idx].azimuth = value
        elif key == 'ambient':
            self.ambient = value
        elif key == 'wa':
            self.wa = self.try_parse_decimal(value)
        elif key == 'wallHeight':
            self.wall_height = self.try_parse_decimal(value)
        elif key == 'baseHeight':
            self.base_height = self.try_parse_decimal(value)
        elif key == 'designer':
            self.author = value
        elif key == 'information':
            self.tagline = value
        elif key == 'pointsToThickness':
            self.pixel_to_thickness = self.try_parse_decimal(value)
        else:
            # print("%s = %s" % (key, value))
            if value in self.replace_vars:
                self.replace_vars[key] = self.replace_vars[value]
            else:
                self.replace_vars[key] = value

    def try_parse_decimal(self, value):
        try:
            return Decimal(value)
        # if its a string lets try to see 
        # if we have createdl a variable for
        # it before
        except InvalidOperation:
            return Decimal(self.replace_vars[value])

    def parse_adjust(self, word):
        if word == "SkyColor":
            sc = SkyColor()
            sc.horizon = self.cur_arc.fill
            sc.color = self.cur_arc.stroke
            self.g_vars.append(sc)
        elif word == "GroundColor":
            g = GroundColor()
            g.color = self.cur_arc.fill
            self.g_vars.append(g)

    def parse_object(self, the_obj):

        type = the_obj['type']
        if type == "Incarnator":
            inc = Incarnator()
            inc.location = self.cur_arc.center
            if 'y' in the_obj:
                inc.location.y += Decimal(the_obj['y'])
            inc.heading = self.cur_arc.heading
            self.incarnators.append(inc)

        elif type == "FreeSolid":
            fs = FreeSolid()
            fs.location = self.cur_arc.center
            if 'y' in the_obj:
                fs.location.y = Decimal(the_obj['y'])

            if 'shape' in the_obj:
                fs.shape = the_obj['shape']

            if 'shotPower' in the_obj:
                fs.power = the_obj['shotPower']

            if 'mass' in the_obj:
                fs.mass = Decimal(the_obj['mass'])

            if 'customGravity' in the_obj:
                fs.relative_gravity = Decimal(the_obj['customGravity'])

            fs.heading = self.cur_arc.heading
            self.free_solids.append(fs)

        elif type == "Goody":
            good = Goody()
            good.location = self.cur_arc.center

            if 'y' in the_obj:
                good.location.y += Decimal(the_obj['y'])

            if 'grenades' in the_obj:
                good.grenades = the_obj['grenades']

            if 'missiles' in the_obj:
                good.missiles = the_obj['missiles']

            if 'shape' in the_obj:
                good.model = the_obj['shape']

            if 'speed' in the_obj:
                good.spin.x = the_obj['speed']

            self.goodies.append(good)
        elif type == "Ramp":
            self.wa = self.last_wa
            ramp = Ramp()
            block = self.blocks.pop()
            arc = self.cur_arc
            if 'deltaY' in the_obj:
                deltaY = Decimal(the_obj['deltaY'])
            else:
                deltaY = 1
            ramp.color = arc.fill

            if 'y' in the_obj:
                y = Decimal(the_obj['y'])
            else:
                y = Decimal(0)
            if 'thickness' in the_obj:
                ramp.thickness = Decimal(the_obj['thickness'])
            else:
                ramp.thickness = block.rounding

            if deltaY == 0:
                block.size.y = ramp.thickness
                block.center.y = y + self.base_height
                block.color = arc.fill
                self.blocks.append(block)
                return

            ramp.base.x = block.center.x
            ramp.base.y = y + self.base_height
            ramp.base.z = block.center.z
            ramp.top.x = block.center.x
            ramp.top.y = y + self.base_height + deltaY
            ramp.top.z = block.center.z

            if arc.heading > 315 or arc.heading <= 45:
                ramp.width = block.size.x

                ramp.base.z -= block.size.z / Decimal(2)
                ramp.top.z += block.size.z / Decimal(2)

            elif arc.heading > 45 and arc.heading <= 135:
                ramp.width = block.size.z

                ramp.base.x += block.size.x / Decimal(2)
                ramp.top.x -= block.size.x / Decimal(2)

            elif arc.heading > 135 and arc.heading <= 225:
                ramp.width = block.size.x

                ramp.base.z += block.size.z / Decimal(2)
                ramp.top.z -= block.size.z / Decimal(2)

            else:
                ramp.width = block.size.z

                ramp.base.x -= block.size.x / Decimal(2)
                ramp.top.x += block.size.x / Decimal(2)

            self.ramps.append(ramp)

    def scale_and_snap(self, dec):
        return (dec / Converter.SCALE).quantize(Converter.SNAP)
