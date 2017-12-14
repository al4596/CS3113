#!/usr/bin/env python3
SPRITESHEET_WIDTH = 512
SPRITESHEET_HEIGHT = 512
SPRITE_SCALE = 144 # this number of pixels corresponds to one unit in OpenGL
def box(top, right, bottom, left):
    return [
        left, top,
        left, bottom,
        right, top,
        right, top,
        left, bottom,
        right, bottom
    ]
def c_array(data):
    return "{\n" + ",\n".join("\t{" + ", ".join(str(f) + "f" for f in a) + "}" for a in data) + "\n};\n"
if __name__ == "__main__":
    # Every state has a different width, height, and position in the spritesheet.
    state_names = []
    state_vertices = []
    state_texture_coordinates = []
    # Get this data from the file.
    with open("Player.txt", "r") as file:
        for line_number, line in enumerate(file, 1):
            # Each line should have six space-separated parts.
            parts = line.split()
            if len(parts) != 6:
                print("Line", line_number, "has an unsupported number of spaces.")
                continue
            # The second part should be an equal sign.
            if parts[1] != "=":
                print("Line", line_number, "has an supported syntax.")
                continue
            # The last four parts should be integers.
            try:
                sprite_x = int(parts[2])
                sprite_y = int(parts[3])
                sprite_width = int(parts[4])
                sprite_height = int(parts[5])
            except ValueError:
                print("Line", line_number, "has a non-integer value.")
                continue
            # Store the name of this state.
            state_names.append(parts[0])
            # Store the vertex array of this state.
            ortho_width_half = sprite_width / SPRITE_SCALE / 2
            ortho_height_half = sprite_height / SPRITE_SCALE / 2
            state_vertices.append(box(ortho_height_half, ortho_width_half, -ortho_height_half, -ortho_width_half))
            # Store the texture coordinate array of this state.
            uv_x = sprite_x / SPRITESHEET_WIDTH
            uv_y = sprite_y / SPRITESHEET_HEIGHT
            uv_width = sprite_width / SPRITESHEET_WIDTH
            uv_height = sprite_height / SPRITESHEET_WIDTH
            state_texture_coordinates.append(box(uv_y, uv_x + uv_width, uv_y + uv_height, uv_x))
    # Write this data to C++ source files.
    with open("PlayerStateEnum.txt", "w") as file:
        file.write("\tenum States {\n")
        for name in state_names:
            file.write("\t\t" + name + ",\n")
        file.write("\t\tNUM_STATES\n")
        file.write("\t};\n")
    with open("PlayerStateData.txt", "w") as file:
        file.write("const float Player::STATE_VERTICES[][12] = ")
        file.write(c_array(state_vertices))
        file.write("const float Player::STATE_TEXTURE[][12] = ")
        file.write(c_array(state_texture_coordinates))
