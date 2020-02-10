#pragma once

#include <string>

#include "envoy/server/filter_config.h"

#include "http_filter.pb.h"
#include "common/common/logger.h"

namespace Envoy {
namespace Http {

class HttpInjectorEncoderFilterConfig {
public:
  HttpInjectorEncoderFilterConfig(const olip::Encoder& proto_config);

  const std::string& script() const { return m_script; }
private:
  const std::string m_script;
};

typedef std::shared_ptr<HttpInjectorEncoderFilterConfig> HttpInjectorEncoderFilterConfigSharedPtr;

class HttpInjectorEncoderFilter : public StreamEncoderFilter, public Logger::Loggable<Logger::Id::filter> {
public:
  HttpInjectorEncoderFilter(const HttpInjectorEncoderFilterConfigSharedPtr);
  ~HttpInjectorEncoderFilter();

  void onDestroy() override;

  FilterHeadersStatus encode100ContinueHeaders(HeaderMap& headers) override;
  FilterHeadersStatus encodeHeaders(HeaderMap& headers, bool end_stream) override;
  FilterDataStatus encodeData(Buffer::Instance& data, bool end_stream) override;
  FilterTrailersStatus encodeTrailers(HeaderMap& trailers) override;
  FilterMetadataStatus encodeMetadata(MetadataMap& metadata_map) override;
  void setEncoderFilterCallbacks(StreamEncoderFilterCallbacks& callbacks) override;
  void encodeComplete() override;

private:
  bool isHtml = false;
  StreamEncoderFilterCallbacks* m_encoder_callbacks;
  const HttpInjectorEncoderFilterConfigSharedPtr config_;
};



} // namespace Http
} // namespace Envoy
