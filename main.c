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

#define PADDING 0      // padding between two sprite
#define PNG_CHANNELS 4 // png channels

static const char *PNG_EXTENSION = ".png";
static const char *CSV_EXTENSION = ".txt";

static int IsFileExtension(const char *filename, const char *ext);
static const char *GetFileExt(const char *filename);
static int FileFilter(const struct dirent *ent);

typedef struct
{
    char *name;
    int x, y;
    int w, h;
    int fmt;
    unsigned char *data;
} Image;

int main(int argc, char **argv)
{

    if (argc < 3)
    {
        printf("[ERROR] Not enough input.\n");
        printf(" \t-- First argument set the dir where you put your images for atlas.\n");
        printf(" \t-- Second argument set the name for the file. (without extension)\n");
        return -1;
    }

    const char *scan_dir = argv[1];

    int output_file_basic_len = strlen(argv[2]);

    int png_ext_len = strlen(PNG_EXTENSION);
    char *output_atlas_file_name = (char *)malloc(png_ext_len + output_file_basic_len + 1);
    strcpy(output_atlas_file_name, argv[2]);
    strcat(output_atlas_file_name, PNG_EXTENSION);

    int csv_ext_len = strlen(CSV_EXTENSION);
    char *output_csv_file_name = (char *)malloc(csv_ext_len + output_file_basic_len + 1);
    strcpy(output_csv_file_name, argv[2]);
    strcat(output_csv_file_name, CSV_EXTENSION);

    printf(" -- Scan directory: %s\n", scan_dir);
    printf(" -- Create an texture atlas named: %s\n", output_atlas_file_name);
    printf(" -- Create an texture atlas csv file named: %s\n", output_csv_file_name);

    char ans;
    printf("[Y/N]: ");
    scanf("%c", &ans);
    if (toupper(ans) == 'N')
    {
        printf("[INFO] Stop create texture atlas...\n");
        return 0;
    }

    // 1. Read elements in this directory
    // ------------------------
    struct dirent **namelist;
    int elements_count;

    printf("[INFO] Start scanning the directory.\n");
    elements_count = scandir(scan_dir, &namelist, FileFilter, alphasort);

    if (elements_count < 0)
    {
        printf("[ERROR] Failed to scan the directory => %s", scan_dir);
        return -1;
    }
    else
    {
        Image *images = (Image *)malloc(elements_count * sizeof(Image));

        stbi_set_flip_vertically_on_load(1);
        for (int i = 0; i < elements_count; i++)
        {
            printf("[INFO] GET: %s...\n", namelist[i]->d_name);
            char *name = (char *)malloc(strlen(scan_dir) + strlen(namelist[i]->d_name) + 2);
            strcpy(name, scan_dir);
            strcat(name, "/");
            strcat(name, namelist[i]->d_name);

            images[i].data = NULL;

            images[i].data = stbi_load(name, &images[i].w, &images[i].h, &images[i].fmt, 0);
            if (images[i].data == NULL)
            {
                printf("[ERROR] Failed to get data from image => %s\n", name);
                continue;
            }
            images[i].name = strdup(namelist[i]->d_name);

            free(namelist[i]);
        }
        free(namelist);

        // 3. Calculate the atlas image size that can fit all sprite
        // ------------------------
        float rq_area = 0.0f;
        for (int i = 0; i < elements_count; i++)
        {
            rq_area += images[i].w * images[i].h;
        }

        float measured_sz = sqrtf(rq_area);
        int image_sz = (int)powf(2.0f, ceilf(log2f(measured_sz)));
        printf("[INFO] Atlas size: %d * %d\n", image_sz, image_sz);

        // 3. Pack the image to larger image, bin packing using `stb_rect_pack`
        // ------------------------
        stbrp_context *context = (stbrp_context *)malloc(sizeof(stbrp_context));
        stbrp_node *nodes = (stbrp_node *)malloc(elements_count * sizeof(stbrp_node));

        stbrp_init_target(context, image_sz, image_sz, nodes, elements_count);
        stbrp_rect *rects = (stbrp_rect *)malloc(elements_count * sizeof(stbrp_rect));

        for (int i = 0; i < elements_count; i++)
        {
            rects[i].id = i;
            rects[i].w = images[i].w + PADDING;
            rects[i].h = images[i].h + PADDING;
        }

        stbrp_pack_rects(context, rects, elements_count);
        for (int i = 0; i < elements_count; i++)
        {
            images[i].x = rects[i].x;
            images[i].y = rects[i].y;
        }

        // 4. Draw the texture atlas
        // ------------------------
        unsigned char *atlas_data = (unsigned char *)calloc(image_sz * image_sz * 4, sizeof(unsigned char));
        memset(atlas_data, 0, image_sz * image_sz * PNG_CHANNELS);

        for (int i = 0; i < elements_count; i++)
        {
            if (rects[i].was_packed)
            {
                for (int y = 0; y < images[i].h; y++)
                {
                    for (int x = 0; x < images[i].w; x++)
                    {

                        int atlas_idx = PNG_CHANNELS * ((y + rects[i].y) * image_sz + (x + rects[i].x));
                        int image_idx = PNG_CHANNELS * (y * images[i].w + x);

                        atlas_data[atlas_idx] = images[i].data[image_idx];         // R
                        atlas_data[atlas_idx + 1] = images[i].data[image_idx + 1]; // G
                        atlas_data[atlas_idx + 2] = images[i].data[image_idx + 2]; // B
                        atlas_data[atlas_idx + 3] = images[i].data[image_idx + 3]; // A
                    }
                }
            }
        }

        stbi_flip_vertically_on_write(1); // Start from bottom-left
        if (0 == stbi_write_png(output_atlas_file_name, image_sz, image_sz, PNG_CHANNELS, atlas_data, image_sz * PNG_CHANNELS))
        {
            printf("[ERROR] Failed to create the texture atlas.\n");
            return -1;
        }
        else
        {
            printf("[INFO] Success to create atlas => %s\n", output_atlas_file_name);
        }

        // 5. Write the file
        // ------------------------
        FILE *atlas_csv_file = fopen(output_csv_file_name, "w");
        if (atlas_csv_file == NULL)
        {
            perror("[ERROR] Failed to create file");
            fclose(atlas_csv_file);
            return -1;
        }

        fprintf(atlas_csv_file, "# Format: name,x,y,width,height\n\n");
        for (int i = 0; i < elements_count; i++)
        {
            fprintf(atlas_csv_file, "%s,%d,%d,%d,%d\n", images[i].name, images[i].x, images[i].y, images[i].w, images[i].h);
        }
        fprintf(atlas_csv_file, "\n# Total count: %d", elements_count);

        printf("[INFO] Success to create csv file => %s\n", output_csv_file_name);

        fclose(atlas_csv_file);

        free(rects);
        free(nodes);
        free(context);

        free(images);

        printf("[INFO] Done...");
        getchar();
    }

#if 0   

        /* Read file string in format: https://stackoverflow.com/questions/4689747/how-to-read-specifically-formatted-data-from-a-file/4689881 */

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

static int IsFileExtension(const char *filename, const char *ext)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return 0;

    if (strcmp(dot, ext) == 0)
        return 1;

    return 0;
}

static const char *GetFileExt(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return NULL;

    return dot;
}

static int FileFilter(const struct dirent *ent)
{
    return (IsFileExtension(ent->d_name, ".png"));
}