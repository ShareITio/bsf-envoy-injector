#include <string>

#include "envoy/registry/registry.h"

#include "http_filter.pb.h"
#include "http_filter.pb.validate.h"
#include "http_filter.h"

namespace Envoy {
namespace Server {
namespace Configuration {

class HttpInjectorEncoderFilterConfig : public NamedHttpFilterConfigFactory, public Logger::Loggable<Logger::Id::filter> {
public:
  HttpInjectorEncoderFilterConfig() {}

  Http::FilterFactoryCb createFilterFactoryFromProto(const Protobuf::Message& proto_config,
                                                     const std::string&,
                                                     FactoryContext& context) override {

    const olip::Encoder& conf = Envoy::MessageUtil::downcastAndValidate<const olip::Encoder&>(
                            proto_config, context.messageValidationVisitor());

    return createFilter(conf, context);
  }

  /**
   *  Return the Protobuf Message that represents your config incase you have config proto
   */
  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{new olip::Encoder()};
  }

  std::string name() const override { return "http-injector"; }

private:
  Http::FilterFactoryCb createFilter(const olip::Encoder& proto_config , FactoryContext&) {
    Http::HttpInjectorEncoderFilterConfigSharedPtr config =
        std::make_shared<Http::HttpInjectorEncoderFilterConfig>(
            Http::HttpInjectorEncoderFilterConfig(proto_config));
    return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
      auto filter = new Http::HttpInjectorEncoderFilter(config);
      callbacks.addStreamEncoderFilter(Http::StreamEncoderFilterSharedPtr{filter});
    };
  }
};

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
REGISTER_FACTORY(HttpInjectorEncoderFilterConfig, NamedHttpFilterConfigFactory);

} // namespace Configuration
} // namespace Server
} // namespace Envoy
