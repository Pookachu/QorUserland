#include "graphics.h"

#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/sys/syscalls.h>
#include <libc/sys/types.h>

#include "libimg.h"

static int FRAME_BUFFER_FD = -1;
static struct Pixel* frame_buffer = 0;

int LIBGRAPHICS_ERROR = 0;

int verify_framebuffer()
{
    if (FRAME_BUFFER_FD < 0)
    {
        LIBGRAPHICS_ERROR = LIBGRAPHICS_UNINITIALIZED_FRAMEBUFFER;
        return -1;
    }

    if (frame_buffer == 0)
    {
        LIBGRAPHICS_ERROR = LIBGRAPHICS_UNMAPPED_FRAMEBUFFER;
        return -1;
    }

    return 0;
}

int init_framebuffer()
{
    FRAME_BUFFER_FD = sys_open("/dev/fb0", O_WRONLY);

    if (FRAME_BUFFER_FD < 0)
    {
        LIBGRAPHICS_ERROR = LIBGRAPHICS_UNABLE_TO_OPEN_FRAMEBUFFER;
        return -1;
    }

    frame_buffer = sys_mmap(0, 640 * 480 * 4, PROT_READ | PROT_WRITE, 0, FRAME_BUFFER_FD, 0);

    if (frame_buffer == 0)
    {
        LIBGRAPHICS_ERROR = LIBGRAPHICS_UNABLE_TO_MAP_FRAMEBUFFER;
        return -1;
    }

    return 0;
}

int close_framebuffer()
{
    if (verify_framebuffer() < 0)
    {
        return -1;
    }

    sys_munmap(frame_buffer, 640 * 480 * 4);

    sys_close(FRAME_BUFFER_FD);

    FRAME_BUFFER_FD = -1;
    frame_buffer = 0;

    return 0;
}

struct Pixel* get_framebuffer()
{
    if (verify_framebuffer() < 0)
    {
        return 0;
    }

    return frame_buffer;
}

int compute_location(int x, int y)
{
    return 640 * y + x;
}

int run_shader(struct Pixel (shader)(int, int))
{
    if (init_framebuffer() < 0)
    {
        return -1;
    }

    run_individual_shader(shader);

    if (close_framebuffer() < 0)
    {
        return -1;
    }

    return 0;
}

int run_individual_shader(struct Pixel (shader)(int, int))
{
    struct Pixel* framebuffer = get_framebuffer();
    if (framebuffer == 0)
    {
        return -1;
    }

    for (int x = 0; x < 640; x++)
    {
        for (int y = 0; y < 480; y++)
        {
            framebuffer[compute_location(x, y)] = shader(x, y);
        }
    }
    
    return 0;
}

int flush_framebuffer()
{
    if (verify_framebuffer() < 0)
    {
        return -1;
    }

    sys_ioctl(FRAME_BUFFER_FD, FB_FLUSH, 0);

    return 0;
}

char* graphics_strerror(int error)
{
    switch (error)
    {
        case LIBGRAPHICS_UNINITIALIZED_FRAMEBUFFER:
            return "Framebuffer Not Initialized";
        case LIBGRAPHICS_UNMAPPED_FRAMEBUFFER:
            return "Framebuffer Not Mapped";
        case LIBGRAPHICS_UNABLE_TO_OPEN_FRAMEBUFFER:
            return "Unable to Open Framebuffer";
        case LIBGRAPHICS_UNABLE_TO_MAP_FRAMEBUFFER:
            return "Unable to Map Framebuffer";
        default:
            return "DEFAULT CASE";
    }
}

void graphics_perror()
{
    assert(LIBGRAPHICS_ERROR);

    eprintf("LibGraphics Error: `%s`\n", graphics_strerror(LIBGRAPHICS_ERROR));
    sys_exit(-1);
}

void blit(struct image_data* data, size_t x, size_t y)
{
    init_framebuffer();

    struct Pixel* fb = get_framebuffer();

    size_t row_length = sizeof(struct Pixel) * data->width;

    for (size_t i = 0; i < data->height; i++)
    {
        memcpy(&fb[compute_location(x, y + i)], data->buffer + row_length * i, row_length);
    }

    flush_framebuffer();
    close_framebuffer();
}