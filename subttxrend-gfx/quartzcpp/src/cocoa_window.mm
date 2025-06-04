/**
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @file cocoa_window.mm
 * Subtec Simulator Player
 */
#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#import "cocoa_window.h"
#include <GLUT/glut.h>
#include <OpenGL/gl.h>

#define ADD_BACKGROUND_IMAGE
//#define USE_TEST_CHECK

@interface SubtitleView : NSOpenGLView
{
    BOOL _updateTexture;
    uint8_t* _data;
    unsigned int _size;
    GLuint _texture;
    NSSize _dimensions;
}
- (void) drawRect: (NSRect) bounds;
- (BOOL) setSubtitleData: (uint8_t*) data withSize: (unsigned int) size andDimensions: (NSSize) dimensions;
- (void) clearSubtitleData;
- (void) releaseBitmapAndTexture;

@property (readonly) BOOL mute;

@end

@implementation SubtitleView

-(void) drawRect: (NSRect) bounds
{
    NSSize viewSize = self.bounds.size;

    //NSLog(@"SubtitleView::drawRect data=%p, width=%f, height=%f, view width=%f, height=%f, ratio=%f, mute=%s", _data, _dimensions.width, _dimensions.height, viewSize.width, viewSize.height, viewSize.width / viewSize.height, _mute ? "true" : "false");

    if (_data != NULL)
    {
#ifdef USE_TEST_CHECK
        _dimensions.width = 1920;
        _dimensions.height = 1080;
        int checkImageWidth = _dimensions.width;
        int checkImageHeight = _dimensions.height;
        GLubyte *checkImage = (GLubyte*) malloc(checkImageHeight * checkImageWidth * 4 * sizeof(GLubyte));

        for (int i = 0; i < checkImageHeight; i++) {
            for (int j = 0; j < checkImageWidth; j++) {
                int c = ((((i&0x80)==0)^((j&0x80))==0))*255;
                unsigned int offset = (i * 4 * checkImageWidth) + (j  * 4);
                *(checkImage + offset) = (GLubyte) 0;
                *(checkImage + offset + 1) = (GLubyte) c;
                *(checkImage + offset + 2) = (GLubyte) 0;
                *(checkImage + offset + 3) = (GLubyte) c; //0xFF
            }
        }
#endif
        glEnable(GL_DOUBLE);
        glEnable(GL_DEPTH);
        glEnable(GL_RGB);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, .0f, 0.0f, 0.0f ); // clear background
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        if (_texture == 0)
        {
            NSLog(@"SubtitleView::drawRect generate texture");
            glGenTextures(1, &_texture);
        }

        if (_updateTexture == TRUE)
        {
            NSLog(@"SubtitleView::drawRect update texture");
            glBindTexture(GL_TEXTURE_2D, _texture);
#ifdef USE_TEST_CHECK
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _dimensions.width, _dimensions.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
#else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _dimensions.width, _dimensions.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
#endif
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            _updateTexture = FALSE;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // Clear the window with current clearing color
        glShadeModel(GL_SMOOTH);
        glEnable(GL_NORMALIZE);
        glPushMatrix();
        {
            glDisable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, _texture);
            glBegin(GL_QUADS); // draw something with the texture on
            {
                /* Flip the co-ordinate system because MacOS origin is in the (vertically) opposite corner to linux */
                glTexCoord2f(0.0, 1.0);
                glVertex2f(-1.0, -1.0);

                glTexCoord2f(1.0, 1.0);
                glVertex2f(1.0, -1.0);

                glTexCoord2f(1.0, 0.0);
                glVertex2f(1.0, 1.0);

                glTexCoord2f(0.0, 0.0);
                glVertex2f(-1.0, 1.0);
            }
            glEnd();
        }
        glPopMatrix();
        glFlush();

#ifdef USE_TEST_CHECK
        free(checkImage);
#endif
    }

}

- (BOOL) setSubtitleData: (uint8_t*) data withSize: (unsigned int) size andDimensions: (NSSize) dimensions
{
    NSLog(@"SubtitleView::setSubtitleData data=%p, dataSize=%u, width=%f, height=%f", data, size, dimensions.width, dimensions.height);

    BOOL ret = TRUE;

    if (_data != NULL)
    {
        free(_data);
        _data = NULL;
    }
    if (data != NULL)
    {
        _data = (uint8_t*)malloc(size);
        if (_data != NULL)
        {
            _size = size;
            memcpy(_data, data, size);
            _updateTexture = TRUE;
            _mute = FALSE;
        }
        else
        {
            ret = FALSE;
        }
        _dimensions = dimensions;
    }
    return ret;
}

- (void) clearSubtitleData
{
    NSLog(@"SubtitleView::clearSubtitleData");

    if (_data != NULL)
    {
        memset(_data, 0x00, _size);
        _updateTexture = TRUE;
        _mute = TRUE;
    }
}

- (void) releaseBitmapAndTexture
{
    NSLog(@"SubtitleView::releaseBitmapAndTexture");

    if (_data != NULL)
    {
        free(_data);
        _data = NULL;
    }
    if (_texture != 0)
    {
        glDeleteTextures(1, &_texture);
    }
}

@end

@interface BackgroundImageView: NSView
{
}
- (void) drawRect: (NSRect) rect;

@property NSImage * backgroundImage;

@end


@implementation BackgroundImageView

- (void) drawRect: (NSRect) rect
{
    if (_backgroundImage != nil)
    {
        [_backgroundImage drawInRect:rect];
    }
}

@end

@interface VideoWindow: NSWindow <NSApplicationDelegate>
{
    BOOL _windowActive;
    BackgroundImageView * _imageView;
    SubtitleView * _subtitleView;
    NSOpenGLContext* _glContext;
    NSSize _aspectRatio;
}

- (BOOL) isActive;
- (id) initializeWindow: (NSRect) bounds;
- (BOOL) setSubtitleData: (uint8_t*) data withSize: (unsigned int) size andDimensions: (NSSize) dimensions;
- (void) muteSubtitles;

@end

static VideoWindow *gCocoaWindow=nil;
static NSApplication* application;

@implementation VideoWindow

- (id) initializeWindow: (NSRect) bounds
{
    self = [super initWithContentRect: bounds
                            styleMask: (NSWindowStyleMaskClosable | NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable)
                              backing: NSBackingStoreBuffered defer: NO screen: nil];

    [application setDelegate:self];
    [self setReleasedWhenClosed:NO];
    _windowActive = TRUE;
    [self setTitle:@"Subtec Test Player"];

    _aspectRatio = bounds.size;

#ifdef ADD_BACKGROUND_IMAGE
    NSImage *image = [[NSImage alloc]initWithContentsOfFile: @"./build/install/usr/local/assets/RDKLogo.png"];
    if (image == nil)
    {
        NSLog(@"image nil");
    }

    _imageView = [[BackgroundImageView alloc] initWithFrame:CGRectMake(0, 0, bounds.size.width, bounds.size.height)];
    _imageView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    _imageView.backgroundImage = image;
    [self.contentView addSubview:_imageView];
#endif

    // Specify the pixel-format attributes.
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFAWindow,
       // NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 32,
        0
    };
    NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];

    _subtitleView = [[SubtitleView alloc] initWithFrame:CGRectMake(0, 0, bounds.size.width, bounds.size.height)
                                            pixelFormat:pixFmt];
    [pixFmt release];
    _subtitleView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [self.contentView addSubview:_subtitleView];

    _glContext = [_subtitleView openGLContext];
    GLint opacity = 0;

    [_glContext setValues: &opacity forParameter: NSOpenGLCPSurfaceOpacity];
    [_glContext makeCurrentContext];

    return self;
}

- (BOOL) windowShouldClose: (id) sender
{
    _windowActive = FALSE;
    return YES;
}

- (void) applicationDidFinishLaunching: (NSNotification *) notification
{
    [self makeMainWindow];
    [self center];
    [self orderFront:self];
}

- (void) applicationWillTerminate: (NSNotification *) notification
{
    if (_subtitleView)
    {
        [_subtitleView releaseBitmapAndTexture];
    }
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication *) application
{
    return YES;
}

- (BOOL) isActive
{
    return _windowActive;
}

- (NSSize) windowWillResize: (NSWindow *) sender toSize: (NSSize) frameSize
{
    sender.contentAspectRatio = NSMakeSize(_aspectRatio.width, _aspectRatio.height);
    return frameSize;
}

- (BOOL) setSubtitleData: (uint8_t*) data withSize: (unsigned int) size andDimensions: (NSSize) dimensions
{
    BOOL ret = TRUE;

    NSLog(@"VideoWindow::setSubtitleData _subtitleView=%p", _subtitleView);

    if (_subtitleView != nil)
    {
        ret = [_subtitleView setSubtitleData:data withSize:size andDimensions:dimensions];
        if (ret)
        {
            _subtitleView.needsDisplay = TRUE;
            [_subtitleView displayIfNeeded];
        }
    }

    return ret;
}

- (void) muteSubtitles
{
    if (_subtitleView != nil)
    {
        if (_subtitleView.mute == false)
        {
            NSLog(@"VideoWindow::muteSubtitles muting subtitles");
            [_subtitleView clearSubtitleData];
            _subtitleView.needsDisplay = TRUE;
            [_subtitleView displayIfNeeded];
        }
    }
}

@end

namespace subttxrend
{
namespace gfx
{

int createAndRunCocoaWindow(int width, int height)
{
    NSRect windowCordinates;
    windowCordinates.size.width = width;
    windowCordinates.size.height = height;
    windowCordinates.origin.x = 0;
    windowCordinates.origin.y = 0;

    application = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    gCocoaWindow = [[VideoWindow alloc] initializeWindow:windowCordinates];

    [gCocoaWindow orderFront:nil];
    [gCocoaWindow setLevel: NSNormalWindowLevel];

    [application run];
    return 0;
}

void setSimulatorWindowTitle(const char *title)
{
    NSString *nsTitle = [NSString stringWithUTF8String:title];
    if( gCocoaWindow )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [gCocoaWindow setTitle:nsTitle];
        });
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            glutSetWindowTitle( [nsTitle cStringUsingEncoding:NSASCIIStringEncoding] );
        });
    }
}

void setSubtitleData(uint8_t * data, unsigned int size, int width, int height)
{
    NSLog(@"Cocoa setBuffer data=%p, size=%u, width=%d, height=%d, bytes per row=%u", data, size, width, height, size / height);

    NSSize dimensions;
    dimensions.width = width;
    dimensions.height = height;

    if (size == 0)
    {
        NSLog(@"Data not allocated!");
    }
    else
    {
        [gCocoaWindow setSubtitleData:data withSize:size andDimensions:dimensions];
    }
}

void muteSubtitles()
{
    [gCocoaWindow muteSubtitles];
}

} // namespace gfx
} // namespace subttxrend