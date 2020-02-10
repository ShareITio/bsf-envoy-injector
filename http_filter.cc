#include <string>

#include "http_filter.h"
#include <iostream>
#include "envoy/server/filter_config.h"
#include "absl/container/fixed_array.h"

namespace Envoy {
namespace Http {

HttpInjectorEncoderFilterConfig::HttpInjectorEncoderFilterConfig(const olip::Encoder& encoder): m_script(encoder.script()) {}


HttpInjectorEncoderFilter::HttpInjectorEncoderFilter(const HttpInjectorEncoderFilterConfigSharedPtr config): config_(config) {}

HttpInjectorEncoderFilter::~HttpInjectorEncoderFilter() {}

void HttpInjectorEncoderFilter::onDestroy() {}

FilterHeadersStatus HttpInjectorEncoderFilter::encode100ContinueHeaders(HeaderMap&) {
  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus HttpInjectorEncoderFilter::encodeHeaders(HeaderMap& headers, bool ) {
  if (headers.ContentType() != NULL) {
    std::string contentType(headers.ContentType()->value().getStringView());

    // only modifies buffer for html documents
    if ( contentType.find("text/html") != std::string::npos) {
      isHtml = true;

      // update content type to fit the script tag to add
      if (headers.ContentLength() != NULL) {
        std::string strContentLength = std::string(headers.ContentLength()->value().getStringView());
        std::int32_t contentLength = std::stoi(strContentLength);
        contentLength += config_->script().size();
        headers.setCopy(LowerCaseString("content-length"), std::to_string(contentLength));
      }
    }
  }

  
  return FilterHeadersStatus::Continue;

}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

FilterDataStatus HttpInjectorEncoderFilter::encodeData(Buffer::Instance& buffer, bool /*endStream*/) {
  if (isHtml) {

    // extract all data in the buffer
    const uint64_t num_slices = buffer.getRawSlices(nullptr, 0);
    absl::FixedArray<Buffer::RawSlice> slices(num_slices);
    buffer.getRawSlices(slices.begin(), num_slices);
    std::string wholeContent("");
    for (const Buffer::RawSlice& input_slice : slices) {
      unsigned char* bytes = static_cast<unsigned char*>(input_slice.mem_);

      std::string s( reinterpret_cast< char const* >(bytes) ) ;
      wholeContent.append(s);
    }

    // prepend the </body> tag by the script given in the config
    std::string newContent = ReplaceAll(wholeContent, "</body>",  this->config_->script() + "</body>");
    newContent = ReplaceAll(newContent, "</BODY>",  this->config_->script() + "</BODY>");
    buffer.drain(buffer.length());
    buffer.add(newContent);
  }
  return FilterDataStatus::Continue;
  
}

FilterTrailersStatus HttpInjectorEncoderFilter::encodeTrailers(HeaderMap& ) {
  return FilterTrailersStatus::Continue;
}
FilterMetadataStatus HttpInjectorEncoderFilter::encodeMetadata(MetadataMap& ) {
  return FilterMetadataStatus::Continue;
}
void HttpInjectorEncoderFilter::setEncoderFilterCallbacks(StreamEncoderFilterCallbacks& callbacks) {
  this->m_encoder_callbacks = &callbacks;
}
void HttpInjectorEncoderFilter::encodeComplete() {}

} // namespace Http
} // namespace Envoy
