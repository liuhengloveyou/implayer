#ifndef IMPLAYER_I_DECODER_H
#define IMPLAYER_I_DECODER_H

#include "core/frame.h"
#include "core/media_file_info.h"

namespace implayer
{
  class IDecoder
  {
  public:
    /**
     * open a video file
     * @param file_path video file path
     * @return 0 if success, otherwise return error code
     */
    virtual int open(const std::string &file_path) = 0;

    /**
     * decode next frame
     * @return a shared_ptr of VideoFrame if success, otherwise return nullptr
     */
    virtual std::shared_ptr<Frame> decodeNextFrame() = 0;

    /**
     * seek to a timestamp quickly and get the video frame
     *
     * @param timestamp the timestamp(us) to seek
     * @return video frame if success, otherwise return nullptr
     */
    virtual std::shared_ptr<Frame> seekFrameQuick(int64_t timestamp) = 0;

    /**
     * seek to a timestamp precisely and get the video frame
     * @param timestamp the timestamp(us) to seek
     * @return video frame if success, otherwise return nullptr
     */
    virtual std::shared_ptr<Frame> seekFramePrecise(int64_t timestamp) = 0;

    /**
     * get the current position of the decoder
     * @return the current position(us)
     */
    virtual int64_t getPosition() = 0;

   /**
     * get the media infomation
     * @return media infomation
     */
    virtual MediaFileInfo getMediaFileInfo() = 0;
  };

}

#endif
