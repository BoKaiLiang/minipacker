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
    int x, y;
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

        stbrp_context* context = (stbrp_context*)malloc(sizeof(stbrp_context));
        stbrp_node* nodes = (stbrp_node*)malloc(n * sizeof(stbrp_node));

        stbrp_init_target(context, 1024, 1024, nodes, n);
        stbrp_rect* rects = (stbrp_rect*)malloc(n * sizeof(stbrp_rect));

        int padding = 2;

        for (int i = 0; i < n; i++) {
            rects[i].id = i;
            rects[i].w = images[i].w + 2 * padding;
            rects[i].h = images[i].h + 2 * padding;
        }

        stbrp_pack_rects(context, rects, n);
        for (int i = 0; i < n; i++) {
            images[i].x = rects[i].x;
            images[i].y = rects[i].y;
        }

        float rq_area = 0.0f;
        for (int i = 0; i < n; i++) {
            rq_area += images[i].w * images[i].h;
        }

        float measured_sz = sqrtf(rq_area);
        int image_sz = (int)powf(2.0f, ceilf(log2f(measured_sz)));

        printf("Atlas w and h: %d\n", image_sz);

        unsigned char* atlas_data = (unsigned char*)calloc(image_sz * image_sz * 4, sizeof(unsigned char));
        memset(atlas_data, 0, image_sz * image_sz * 4);
        
        for (int i = 0; i < n; i++) {

            // printf("name: %s. rect: %i >> x: %i, y: %i, w: %i, h: %i\n", images[i].name, rects[i].id, rects[i].x, rects[i].y, rects[i].w, rects[i].h);

            if (rects[i].was_packed) {
                for (int y = 0; y < images[i].h; y++) {
                    for (int x = 0; x < images[i].w; x++) {
                        
                        int atlas_idx = 4 * ((y + rects[i].y) * image_sz +  (x + rects[i].x));
                        int image_idx = 4 * (y * images[i].w + x);

                        atlas_data[atlas_idx] = images[i].data[image_idx];
                        atlas_data[atlas_idx + 1] = images[i].data[image_idx + 1];
                        atlas_data[atlas_idx + 2] = images[i].data[image_idx + 2];
                        atlas_data[atlas_idx + 3] = images[i].data[image_idx + 3];
                    }   
                }
            }
        }
        
#endif

        // 在 (10, 10)為原點的地方畫出 128 * 128的正方形
/*
        for (int y = 0; y < 128; y += 1) {
            for (int x = 0; x < 128; x += 1) {

                int index = 4 * ((y + 128) * 1024 + (x + 128));

                atlas_data[index] = 127; 
                atlas_data[index + 1] = 0; 
                atlas_data[index + 2] = 255; 
                atlas_data[index + 3] = 255; 
            }
        }
*/
#if 0      
        stbi_write_png("cute.png", image_sz, image_sz, 4, atlas_data, image_sz * 4);

        FILE* csv = fopen("cute.txt", "w");
        if (csv == NULL) {
            perror("Failed to create file");
            fclose(csv);
            return -1;
        }

        fprintf(csv, "name,x,y,width,height\n");
        for (int i = 0; i < n; i++) {
            fprintf(csv, "%s,%d,%d,%d,%d\n", images[i].name, images[i].x, images[i].y, images[i].w, images[i].h);
        }

        /* Read file string in format: https://stackoverflow.com/questions/4689747/how-to-read-specifically-formatted-data-from-a-file/4689881 */

        fclose(csv);
#endif

        free(rects);
        free(nodes);
        free(context);

        free(images);

    }   

#if 0   
        // Serialize file here

        // test reading file
        FILE* f = fopen("cute.txt", "r");
        if (f == NULL) {
            perror("Failed to read file");
            fclose(f);
            return -1;
        }

        int line_count = 0;
        while (1) {
            char line[256] = { 0 };
            int res = fscanf(f, "%s", line);
            if (res == EOF)
                break;
            
            line_count += 1;
            printf("line %d: %s\n", line_count, line);

            char* tok;

            char name[256];
            int x, y, w, h;
            /*
            tok = strtok(line, ",");
            while (tok != NULL) {
                printf("%s\n", tok);
                tok = strtok(NULL, ",");
            }
            */
            res = sscanf(line, "%[^,],%d,%d,%d,%d", name, &x, &y, &w, &h);
            if (res == 5) {
                printf("image: %s, x: %d, y: %d, width: %d, height: %d\n", name, x, y, w, h);
            }
        }

        fclose(f);
    
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