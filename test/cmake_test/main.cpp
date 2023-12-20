#include <boost/http_proto.hpp>

int main() {
  boost::http_proto::request request;
  request.set_payload_size(1337);
  if (request.payload_size() != 1337) {
    throw;
  }
}
