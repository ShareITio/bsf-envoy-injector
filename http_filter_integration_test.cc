#include "test/integration/http_integration.h"

namespace Envoy {
class HttpFilterSampleIntegrationTest : public HttpIntegrationTest,
                                        public testing::TestWithParam<Network::Address::IpVersion> {
public:
  HttpFilterSampleIntegrationTest()
      : HttpIntegrationTest(Http::CodecClient::Type::HTTP1, GetParam(), realTime()) {}
  /**
   * Initializer for an individual integration test.
   */
  void SetUp() override { initialize(); }

  void initialize() override {
    config_helper_.addFilter(
      "name: sample\n"
      "config:\n"
      "  script: <script type='text/javascript' src='example.com'><script>"
    );
    HttpIntegrationTest::initialize();
  }
};

INSTANTIATE_TEST_SUITE_P(IpVersions, HttpFilterSampleIntegrationTest,
                        testing::ValuesIn(TestEnvironment::getIpVersionsForTest()));

TEST_P(HttpFilterSampleIntegrationTest, Test1) {
  Http::TestHeaderMapImpl headers{{":method", "GET"}, {":path", "/"}, {":authority", "host"}};

  IntegrationCodecClientPtr codec_client;
  FakeHttpConnectionPtr fake_upstream_connection;
  FakeStreamPtr request_stream;

  codec_client = makeHttpConnection(lookupPort("http"));
  auto response = codec_client->makeHeaderOnlyRequest(headers);
  ASSERT_TRUE(fake_upstreams_[0]->waitForHttpConnection(*dispatcher_, fake_upstream_connection,
                                                        std::chrono::milliseconds(5)));
  ASSERT_TRUE(fake_upstream_connection->waitForNewStream(*dispatcher_, request_stream));

  ASSERT_TRUE(request_stream->waitForEndStream(*dispatcher_));

  request_stream->encodeHeaders(Http::TestHeaderMapImpl {{":status", "200"}, {"Content-type", "text/html"}}, false);
  
  request_stream->encodeData("<html><body></body></html>", true);
  ASSERT_TRUE(request_stream->waitForEndStream(*dispatcher_));

  response->waitForEndStream();

  codec_client->close();
}
} // namespace Envoy
