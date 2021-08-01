#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dirent.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

static int IsFileExtension(const char* filename, const char* ext);
static const char* GetFileExt(const char* filename);
static int FileFilter(const struct dirent* ent);

typedef struct {
    char* name;
    int w, h;
    int fmt;
    unsigned char* data;
} Image;

int main(int argc, char** argv) {

#if 0
    if (argc < 2) {
        printf("[ERROR] No input.");
        return -1;
    }
#endif
    const char* dir_name = "D:/assets/textures/kenney_animalpackredux/PNG/Square";

#if 0
    DIR* dir = opendir(dir_name);
    if (!dir) {
        printf("[ERROR] Failed to open directory.");
        return -1;
    }
#endif

#if 1
    struct dirent **namelist;
    int n;

    n = scandir(dir_name, &namelist, FileFilter, alphasort);
    if (n < 0)
       perror("scandir");
    else {
        Image* images = (Image*)malloc(n * sizeof(Image));
        
        for (int i = 0; i < n; i++) {
            char* name = (char*)malloc(strlen(dir_name) + strlen(namelist[i]->d_name) + 2);
            strcpy(name, dir_name);
            strcat(name, "/");
            strcat(name, namelist[i]->d_name);
            
            images[i].data = NULL;

            images[i].data = stbi_load(name, &images[i].w, &images[i].h, &images[i].fmt, 0);
            if (images[i].data == NULL) {
                printf("[ERROR] failed to get data from %s\n", name);
                continue;
            }
            images[i].name = strdup(namelist[i]->d_name);

            free(namelist[i]);
        }
        free(namelist);

        // stbi_write_png("cute.png", images[0].w, images[0].h, images[0].fmt, images[0].data, 0);

        stbrp_context* context = (stbrp_context*)malloc(sizeof(stbrp_context));
        stbrp_node* nodes = (stbrp_node*)malloc(n * sizeof(stbrp_node));

        stbrp_init_target(context, 1024, 1024, nodes, n);
        stbrp_rect* rects = (stbrp_rect*)malloc(n * sizeof(stbrp_rect));

        for (int i = 0; i < n; i++) {
            rects[i].id = i;
            rects[i].w = images[i].w;
            rects[i].h = images[i].h;
        }

        stbrp_pack_rects(context, rects, n);

        for (int i = 0 ; i < n; i++) {
            printf("rect: %i >> x: %i, y: %i, w: %i, h: %i\n", rects[i].id, rects[i].x, rects[i].y, rects[i].w, rects[i].h);
        }

        unsigned char* atlas_data = (unsigned char*)calloc(1024 * 1024 * 4, sizeof(unsigned char));
        memset(atlas_data, 0, 1024 * 1024 * 4);
#if 0
        for (int i = 0; i < 1; i++) {

            if (rects[i].was_packed) {
                for (int y = 0; y < images[i].h; y++) {
                    for (int x = 0; x < images[i].w; x++) {
                        atlas_data[(rects[i].y + y) * 1024 + (rects[i].x + x)] = images[i].data[y * images[i].w + x];
                    }   
                }
            }
        }
#endif

        for (int i = 0; i < (images[0].w * images[0].h); i += 4) {
            atlas_data[i] = images[0].data[i];
            atlas_data[i + 1] = images[0].data[i + 1];
            atlas_data[i + 2] = images[0].data[i + 2];
            atlas_data[i + 3] = images[0].data[i + 3];
        }

        stbi_write_png("cute.png", 1024, 1024, 4, atlas_data, 0);

        free(rects);
        free(nodes);
        free(context);

    }
#else

    unsigned char* atals_data = (char*)malloc(1024 * 1024 * sizeof(unsigned char) * 4);

    for (int i = 0; i < (512 * 1024); i += 4) {
            atals_data[i] = 255;
            atals_data[i + 1] = 0;
            atals_data[i + 2] = 0;
            atals_data[i + 3] = 255;
        }

    stbi_write_png("atalas.png", 1024, 1024, 4, atals_data, 1024);
#endif

    return 0;
}

static int IsFileExtension(const char* filename, const char* ext) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return 0;
    
    if (strcmp(dot, ext) == 0)
        return 1;

    return 0;
}

static const char* GetFileExt(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return NULL;

    return dot;
}

static int FileFilter(const struct dirent* ent) {
    return (IsFileExtension(ent->d_name, ".png"));
}