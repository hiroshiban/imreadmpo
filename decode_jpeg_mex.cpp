// decode_jpeg_mex.cpp
//
// A high-performance MEX function to decode a JPEG image from a memory buffer.
// This function uses libjpeg to achieve native speed.
//
// Compilation command in MATLAB: compile_mex.m
//
//
// Created    : "2025-10-03 16:06:50 ban"
// Last Update: "2025-10-03 16:14:41 ban"

#include "mex.h"
#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>

// Custom error handler for libjpeg
struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

void my_error_exit(j_common_ptr cinfo) {
  my_error_mgr* myerr = (my_error_mgr*) cinfo->err;
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message)(cinfo, buffer);
  mexErrMsgIdAndTxt("MyToolbox:jpegError", "libjpeg error: %s", buffer);
  longjmp(myerr->setjmp_buffer, 1);
}

// The main MEX function entry point
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

  // 1. check the input variables
  if (nrhs != 1) {
    mexErrMsgIdAndTxt("MyToolbox:decode_jpeg_mex:nrhs", "One input required (uint8 vector).");
  }
  if (!mxIsUint8(prhs[0])) {
    mexErrMsgIdAndTxt("MyToolbox:decode_jpeg_mex:notUint8", "Input must be a uint8 vector.");
  }
  if (mxGetNumberOfDimensions(prhs[0]) > 2 || (mxGetM(prhs[0]) != 1 && mxGetN(prhs[0]) != 1)) {
    mexErrMsgIdAndTxt("MyToolbox:decode_jpeg_mex:notVector", "Input must be a vector.");
  }

  // 2. preparing libjpeg
  unsigned char *jpeg_buffer = (unsigned char *)mxGetData(prhs[0]);
  unsigned long jpeg_size = mxGetNumberOfElements(prhs[0]);

  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    mexErrMsgIdAndTxt("MyToolbox:decode_jpeg_mex:jpegError", "JPEG decompression failed.");
    return;
  }

  jpeg_create_decompress(&cinfo);

  // 3. setting memory content, not file, as a data source
  jpeg_mem_src(&cinfo, jpeg_buffer, jpeg_size);

  // 4. reading JPEG headers and get the image information
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  int width = cinfo.output_width;
  int height = cinfo.output_height;
  int num_channels = cinfo.output_components;

  if (num_channels != 3) {
    jpeg_destroy_decompress(&cinfo);
    mexErrMsgIdAndTxt("MyToolbox:decode_jpeg_mex:notRGB", "Only 3-channel (RGB) JPEGs are supported.");
  }

  // 5. generating the MATLAB output matrix [height, width, channels(RGB)]
  // following the MATLAB matrix conventions (column-major)
  mwSize dims[3] = {(mwSize)height, (mwSize)width, (mwSize)num_channels};
  plhs[0] = mxCreateNumericArray(3, dims, mxUINT8_CLASS, mxREAL);
  unsigned char *output_buffer = (unsigned char *)mxGetData(plhs[0]);

  // 6. decoding line by line and write the content to the MATLAB matrix
  // with preparing a temporal buffer since libjpeg returns data line-by-line
  unsigned char *row_pointer[1];
  row_pointer[0] = new unsigned char[width * num_channels];

  // writing data channel-by-channel as MATLAB takes column-major policy
  unsigned char* r_ptr = output_buffer;
  unsigned char* g_ptr = output_buffer + height * width;
  unsigned char* b_ptr = output_buffer + 2 * height * width;

  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, row_pointer, 1);
    // copying one line of data to the correct position for each RGB channel
    for (int i = 0; i < width; i++) {
      r_ptr[cinfo.output_scanline - 1 + i * height] = row_pointer[0][i * 3];
      g_ptr[cinfo.output_scanline - 1 + i * height] = row_pointer[0][i * 3 + 1];
      b_ptr[cinfo.output_scanline - 1 + i * height] = row_pointer[0][i * 3 + 2];
    }
  }

  // 7. clean up
  delete[] row_pointer[0];
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
}
