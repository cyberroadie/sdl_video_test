//
// Created by Olivier Van Acker on 07/03/2016.
//
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <SDL.h>

int getVideoStream(const AVFormatContext *pFormatCtx, AVCodecContext **pCodecCtxOrig) {
    int videoStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            (videoStream) = i;
            break;
        }
    }

    if ((videoStream) == -1) {
        fprintf(stderr, "Didn't find a video stream");
        return videoStream;
    }

    // Get a pointer to the codec context for the video stream
    (*pCodecCtxOrig) = pFormatCtx->streams[(videoStream)]->codec;

    return videoStream;
}

int main(int argc, char *argv[]) {

    fprintf(stdout, "Starting..");

    av_register_all();

    AVFormatContext *pFormatCtx = NULL;

    // Open video file
    if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0) {
        fprintf(stderr, "Couldn't open file");
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        fprintf(stderr, "Couldn't find stream information");
        return -1; // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);

    AVCodecContext *pCodecCtxOrig = NULL;
    AVCodecContext *pCodecCtx = NULL;

    // Find the first video streamint videoStream;
    int videoStream;
    if ((videoStream = getVideoStream(pFormatCtx, &pCodecCtxOrig))) {
        fprintf(stderr, "Can not find video stream");
        exit(1);
    }

    // Find the decoder for the video stream
    AVCodec *pCodec = NULL;
    pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        exit(1); // Codec not found
    }

    // Copy context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
        fprintf(stderr, "Couldn't copy codec context");
        exit(1);
    }

    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        fprintf(stderr, "Couldn't open codec - %s", SDL_GetError());
        exit(1);
    }

    // Allocate video frame
    AVFrame *pFrame = NULL;
    pFrame = av_frame_alloc();

    // Initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Window *gWindow = SDL_CreateWindow("My Game Window",
                                           SDL_WINDOWPOS_UNDEFINED,
                                           SDL_WINDOWPOS_UNDEFINED,
                                           pCodecCtx->width,
                                           pCodecCtx->height,
                                           SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);

    if (!gWindow) {
        fprintf(stderr, "SDL: could not set video mode - exiting\n");
        exit(1);
    }

    SDL_RendererInfo info;
    SDL_Renderer *renderer = SDL_CreateRenderer(gWindow, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Couldn't set create renderer: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_GetRendererInfo(renderer, &info);
    printf("Using %s rendering\n", info.name);

    SDL_Surface *gScreenSurface = SDL_GetWindowSurface(gWindow);

    // Read frames
    AVPacket packet;
    int frameFinished;
    int frameCount = 0;

    // Read frames
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {

            // Create texture
            SDL_Texture *bmp = SDL_CreateTexture(renderer,
                                                 SDL_PIXELFORMAT_YV12,
                                                 SDL_TEXTUREACCESS_STATIC,
                                                 pCodecCtx->width,
                                                 pCodecCtx->height);




            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            if (frameFinished) {
                frameCount++;
                fprintf(stdout, "Reading frame %i finished\n", frameCount);

                SDL_UpdateYUVTexture(bmp,
                                     NULL,
                                     pFrame->data[0],
                                     pFrame->linesize[0],
                                     pFrame->data[1],
                                     pFrame->linesize[1],
                                     pFrame->data[2],
                                     pFrame->linesize[2]);

                SDL_Rect renderQuad = {0, 0, pCodecCtx->height, pCodecCtx->width};

                renderQuad.w = pCodecCtx->width;
                renderQuad.h = pCodecCtx->height;

//                SDL_RenderCopy(renderer, bmp, NULL, NULL);
                SDL_Point point;
                point.x = pCodecCtx->width;
                point.y = pCodecCtx->height;


                SDL_RenderCopyEx(renderer,
                                 bmp,
                                 NULL,
                                 &renderQuad,
                                 90,
                                 NULL,
                                 SDL_FLIP_NONE);

                SDL_RenderPresent(renderer);
                SDL_Delay(20);

            }
        }
    }

    // Free the YUV frame
    av_frame_free(&pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);
    avcodec_close(pCodecCtxOrig);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    SDL_FreeSurface(gScreenSurface);
    gScreenSurface = NULL;

    //Destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    //Quit SDL subsystems
    SDL_Quit();

    return 0;

}

