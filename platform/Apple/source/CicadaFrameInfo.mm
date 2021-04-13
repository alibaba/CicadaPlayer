

#import "CicadaFrameInfo.h"


@interface CicadaFrameInfo ()
@end

@implementation CicadaFrameInfo

- (instancetype)init {
    if (self = [super init]) {

        self.frameType = CicadaFrameType::Cicada_FrameType_Unknown;
        self.pts = 0;
        self.duration = 0;
        self.key = NO;
        self.timePosition = 0;

        self.audio_format = CicadaSampleFormat::CICADA_SAMPLE_FMT_NONE;
        self.audio_nb_samples = 0;
        self.audio_channels = 0;
        self.audio_sample_rate = 0;
        self.audio_channel_layout = 0;
        self.audio_data_lineNum = 0;
        self.audio_data = nil;
        self.audio_data_lineSize = 0;

        self.video_format = CicadaPixelFormat::CICADA_PIX_FMT_NONE;
        self.video_width = 0;
        self.video_height = 0;
        self.video_rotate = 0;
        self.video_dar = 0;
        self.video_crop_top = 0;
        self.video_crop_bottom = 0;
        self.video_crop_left = 0;
        self.video_crop_right = 0;
        self.video_colorRange = 0;
        self.video_colorSpace = 0;
        self.video_data_lineNum = 0;
        self.video_data_lineSize = nil;
        self.video_data_addr = nil;
        self.video_pixelBuffer = nil;
    }
    return self;
}

@end

