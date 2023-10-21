# define M_PI           3.14159265358979323846

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>


uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
    return (a << 24) + (b << 16) + (g << 8) + r;
}

void unpack_color(const uint32_t& color, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    r = (color >> 0) & 255;
    g = (color >> 8) & 255;
    b = (color >> 16) & 255;
    a = (color >> 24) & 255;
}

void draw_rectangle(std::vector<uint32_t>& img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
    assert(img.size() == img_w * img_h);
    for (size_t i = 0; i < w; i++) {
        for (size_t j = 0; j < h; j++) {
            size_t cx = x + i;
            size_t cy = y + j;
            if (cx >= img_w || cy >= img_h) continue;
            img[cx + cy * img_w] = color;
        }
    }
}

void drop_ppm_image(const std::string filename, const std::vector<uint32_t>& image, const size_t w, const size_t h) {
    assert(image.size() == w * h);
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << w << " " << h << "\n255\n";
    for (size_t i = 0; i < h * w; ++i) {
        uint8_t r, g, b, a;
        unpack_color(image[i], r, g, b, a);
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }
    ofs.close();
}

int main()
{
    const size_t win_w = 1024; // image width
    const size_t win_h = 512; // image height
    std::vector<uint32_t> framebuffer(win_w * win_h, pack_color(255,255,255)); // the image itself, initialized to red

    const size_t map_w = 16;
    const size_t map_h = 16;

    const float player_x = 2;
    const float player_y = 2;
    const float player_a = 1.523; //player angle view
    const float fov = M_PI / 3;

    const char map[] =  "0000000000000000"\
                        "0          0   0"\
                        "0              0"\
                        "0  0000  0000000"\
                        "0  0        0  0"\
                        "0  0        0  0"\
                        "0  0        0  0"\
                        "0      00      0"\
                        "0      00      0"\
                        "0  0        0000"\
                        "0  0        0  0"\
                        "0  0        0  0"\
                        "0000000  0000  0"\
                        "0              0"\
                        "0              0"\
                        "0000000000000000"; //game map

    assert(sizeof(map) == map_w * map_h + 1); // +1 for the null terminated string

    

    const size_t rect_w = win_w / (map_w * 2);
    const size_t rect_h = win_h / map_h;
    for (size_t j = 0; j < map_h; j++) { // draw the map
        for (size_t i = 0; i < map_w; i++) {
            if (map[i + j * map_w] == ' ') continue; // skip empty spaces
            size_t rect_x = i * rect_w;
            size_t rect_y = j * rect_h;
            draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, pack_color(0, 255, 255));
        }
    }

    //Draw player on the map
    draw_rectangle(framebuffer, win_w, win_h, player_x*rect_w, player_y*rect_h, 4, 4, pack_color(0, 0, 0));

    //Draw the fov cone and the 3D view
    for (float i=0; i < win_w / 2; i++) { 
        float angle = player_a - fov / 2 + fov * i / float(win_w / 2);

        for (float c = 0; c < 20; c += .05) {
            float cx = player_x + c * cos(angle);
            float cy = player_y + c * sin(angle);

            size_t pix_x = cx * rect_w;
            size_t pix_y = cy * rect_h;

            framebuffer[pix_x + pix_y * win_w] = pack_color(160, 160, 160);

            if (map[int(cx) + int(cy) * map_w] != ' ') { //happens when the ray touches a wall, drawing the vertical column to create the "3D"
                size_t column_height = win_h / c;
                draw_rectangle(framebuffer, win_w, win_h, win_w / 2 + i, win_h / 2 - column_height / 2, 1, column_height, pack_color(0, 255, 255));

                break;
            }
        }
    }

    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    return 0;
}
