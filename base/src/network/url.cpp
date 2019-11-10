/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "base/network/url.h"

#include <stdlib.h>
#include <string>

namespace Base {

Url::Url() {
  m_hostname[0] = '\0';
  m_service[0] = '\0';
}

Url::Url(const AddressIPv4 &address, u16 port) {
  snprintf(m_service, kServiceMax, "%d", port);
  snprintf(m_hostname, kHostnameMax, "%d.%d.%d.%d", address.GetA(),
           address.GetB(), address.GetC(), address.GetD());
}

Url::Url(const char *string) {
  memset(m_hostname, 0, kHostnameMax);
  memset(m_service, 0, kServiceMax);

  std::string address = string;
  std::size_t p_pos = address.find(':', 0) + 1;

  std::size_t hostname_length = p_pos - 1;
  if(hostname_length > kHostnameMax)
    hostname_length = kHostnameMax;
  std::size_t service_length = address.size() - p_pos;
  if(service_length > kServiceMax)
    service_length = kServiceMax;

  strncpy(m_hostname, string, hostname_length);
  strncpy(m_service, address.substr(p_pos).c_str(), service_length);
}

Url::Url(const char *string, u16 port) {
  memset(m_hostname, 0, kHostnameMax);
  memset(m_service, 0, kServiceMax);

  std::size_t hostname_length = strlen(string);
  if(hostname_length > kHostnameMax)
    hostname_length = kHostnameMax;
  strncpy(m_hostname, string, hostname_length);
  snprintf(m_service, kServiceMax, "%d", port);
}

Url::Url(const char *hostname, const char *service) {
  memset(m_hostname, 0, kHostnameMax);
  memset(m_service, 0, kServiceMax);

  std::size_t hostname_length = strlen(hostname);
  if(hostname_length > kHostnameMax)
    hostname_length = kHostnameMax;
  std::size_t service_length = strlen(service);
  if(service_length > kServiceMax)
    service_length = kServiceMax;

  strncpy(m_hostname, hostname, hostname_length);
  strncpy(m_service, service, service_length);
}

Url::Url(const Socket::Address &addr) {
  addr.GetHostname(m_hostname, kHostnameMax);
  addr.GetService(m_service, kServiceMax);
}

u16 Url::GetPort() const { return atoi(m_service); }

} // namespace Base
