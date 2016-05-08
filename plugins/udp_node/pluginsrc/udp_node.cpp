/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "udp_node.h"

#include "link/plugin_log.h"
#include "base/core/str.h"
#include "base/core/assert.h"
#include "base/math/crc.h"

#include "rush/rush.h"

#include "json_utils.h"
#include "common/json/json_reader.h"
#include "common/json/json_writer.h"
#include "tinyxml2.h"

const char *SimplePlugin::Name = "udp_node";
const char *SimplePlugin::Version = "0.1";

rush_t g_ctx = 0;
RushConfig g_config = {};

SimplePlugin *SimplePlugin::CreatePlugin() { return new Link::UdpNode(); }

void SimplePlugin::DestroyPlugin(SimplePlugin *plugin) { delete plugin; }

namespace Link {

void OnStartup(rush_t ctx, u32 addr, u16 port, void *data) {}

void OnConnectivity(rush_t ctx, endpoint_t endpoint, Connectivity info,
                    void *data) {
  UdpNode *plugin = static_cast<UdpNode *>(data);
  if(info == kRushConnected || info == kRushEstablished) {
    PLUGIN_INFO("%p connected", endpoint);
    plugin->connection.push_back(endpoint);
  } else if(info == kRushConnectionFailed || info == kRushDisconnected) {
    PLUGIN_INFO("%p disconnected %s", endpoint,
                info == kRushConnectionFailed ? "conn failed" : "disconn");
    for(std::vector<endpoint_t>::iterator it = plugin->connection.begin();
        it != plugin->connection.end();) {
      if(*it == endpoint) {
        it = plugin->connection.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void Unpack(rush_t ctx, endpoint_t endpoint, rush_sequence_t id,
            const void *buffer, u16 nbytes, void *data) {
  RushConnection info = {};
  rush_connection_info(ctx, endpoint, &info);
  Base::Url url = Base::Url(Base::AddressIPv4(info.address), info.port);

  //	PLUGIN_INFO("%p unpack from %d.%d.%d.%d:%d via %p id %d. data: %d", ctx,
  //		url.GetAddress().GetA(), url.GetAddress().GetB(),
  // url.GetAddress().GetC(), url.GetAddress().GetD(), url.GetPort(),
  //		info.handle, id , nbytes);
}
void OutOfOrderUnpack(rush_t ctx, endpoint_t endpoint, rush_sequence_t id,
                      const void *buffer, u16 nbytes, void *data) {
  // PLUGIN_WARN("%p lost packet of id %d", endpoint, id);
}

u16 Pack(rush_t ctx, endpoint_t endpoint, rush_sequence_t id, void *buffer,
         u16 nbytes, void *data) {
  // PLUGIN_INFO("%p	 pack %d %d", endpoint, id, nbytes);
  RushConnection info = {};
  rush_connection_info(ctx, endpoint, &info);
  //	Base::Url url = Base::Url(Base::AddressIPv4(info.address), info.port);
  //	PLUGIN_INFO("%p		 pack to %d.%d.%d.%d:%d via %p id %d. data: %d",
  // ctx,
  //		url.GetAddress().GetA(), url.GetAddress().GetB(),
  // url.GetAddress().GetC(), url.GetAddress().GetD(), url.GetPort(),
  //		info.handle, id , nbytes);
  return nbytes;
}

bool ConnectCmd::OnCommand(const std::string &query,
                           const std::string &post_data,
                           std::string *response_data) {
  PLUGIN_INFO("connect with %s", query.c_str());
  Link::JsonReader reader;
  if(!reader.Parse(post_data.c_str(), post_data.length())) {
    *response_data = "{\"status\": \"Failed to parse data.\"}";
    return true;
  }

  std::string address;
  if(!reader.Read("address", &address)) {
    *response_data = "{\"status\": \"Failed to read address.\"}";
    return true;
  }

  if(!address.empty()) {
    Base::Url url(address.c_str());
    // Base::Url url(Base::AddressIPv4::kLocalhost, g_port);
    PLUGIN_INFO("connecting with %d.%d.%d.%d:%d", url.GetAddress().GetA(),
                url.GetAddress().GetB(), url.GetAddress().GetC(),
                url.GetAddress().GetD(), url.GetPort());
    BASE_ASSERT(false, "need to have the punch address resolved");
    endpoint_t endpoint = nullptr; // rush_open(m_ctx, url);
    if(endpoint == 0) {
      PLUGIN_ERROR("failed creating endpoint for %d.%d.%d.%d:%d",
                   url.GetAddress().GetA(), url.GetAddress().GetB(),
                   url.GetAddress().GetC(), url.GetAddress().GetD(),
                   url.GetPort());
      *response_data = "{\"status\": \"Failed to create an endpoint.\"}";
      return true; // todo: return result and true.
    }
    *response_data = "{\"status\": \"Success\"}";
    return true; // todo: return result.
  }
  return false;
}

bool DisconnectCmd::OnCommand(const std::string &query,
                              const std::string &post_data,
                              std::string *response_data) {
  PLUGIN_INFO("disconnect with %s", query.c_str());
  Link::JsonReader reader;
  if(!reader.Parse(post_data.c_str(), post_data.length())) {
    *response_data = "{\"status\": \"Failed to parse data.\"}";
    return true;
  }

  u64 handle;
  if(!reader.Read("handle", &handle)) {
    *response_data = "{\"status\": \"Failed to read handle.\"}";
    return true;
  }

  rush_close(m_ctx, reinterpret_cast<endpoint_t>(handle));
  *response_data = "{\"status\": \"Success\"}";
  return true;
}

bool ListConnectionsCmd::OnCommand(const std::string &query,
                                   const std::string &port_data,
                                   std::string *response_data) {
  PLUGIN_INFO("list %s", query.c_str());

  return false;
}

bool RegulationCmd::OnCommand(const std::string &query,
                              const std::string &post_data,
                              std::string *response_data) {
  PLUGIN_INFO("regulation %s", post_data.c_str());

  Link::JsonReader reader;
  if(!reader.Parse(post_data.c_str(), post_data.length())) {
    *response_data = "{\"status\": \"Failed to parse data.\"}";
    return true;
  }

  std::string action;
  if(!reader.Read("action", &action)) {
    *response_data = "{\"status\": \"Failed to read action attribute.\"}";
    return true;
  }

  std::string stream_type;
  if(!reader.Read("stream", &stream_type)) {
    *response_data = "{\"status\": \"Failed to read stream attribute.\"}";
    return true;
  }

  if(stream_type != "upstream" && stream_type != "downstream") {
    *response_data = "{\"status\": \"Unknown stream type.\"}";
    return true;
  }

  RushStreamType stream =
      stream_type == "upstream" ? kRushUpstream : kRushDownstream;

  if(action == "enable") {
    u16 bps = 0;
    if(!reader.Read("bps", &bps)) {
      *response_data = "{\"status\": \"Failed to read bps attribute.\"}";
      return true;
    }
    if(0 != rush_limit(m_ctx, stream, bps)) {
      *response_data = "{\"status\": \"Failed to enable regulation\"}";
      return true;
    }
    *response_data = "{\"status\": \"Success\"}";
    return true;
  } else if(action == "disable") {
    if(0 != rush_limit(m_ctx, stream, -1)) {
      *response_data = "{\"status\": \"Failed to disable regulation\"}";
      return true;
    }
    *response_data = "{\"status\": \"Success\"}";
    return true;
  } else if(action == "info") {
    u16 bytes_per_second = 0;
    if(0 != rush_limit_info(m_ctx, stream, &bytes_per_second)) {
      *response_data = "{\"status\": \"Failed to get info.\"}";
      return true;
    }

    Link::JsonWriter writer(*response_data);
    writer.Write("status", "Success");
    writer.Write("enable", bytes_per_second != (u16)-1);
    writer.Write("bytes_per_second", bytes_per_second);
    writer.Finalize();
    return true;
  }

  return true;
}

struct EndpointInfo {
  endpoint_t handle;
  u32 rtt;
  u32 address;
  u16 port;
  u16 upstream_bps;
  u16 downstream_bps;
  u16 packet_loss;
  rush_time_t send_interval;
};

template <> inline void JsonWriter::AppendValue(const EndpointInfo &info) {
  JsonWriter writer(m_destination);
  writer.Write("handle", reinterpret_cast<u64>(info.handle));
  writer.Write("rtt", info.rtt);
  writer.Write("upstream_bps", info.upstream_bps);
  writer.Write("downstream_bps", info.downstream_bps);
  writer.Write("address", info.address);
  writer.Write("port", info.port);
  writer.Write("packet_loss", info.packet_loss);
  writer.Write("send_interval", info.send_interval);
  writer.Finalize();
}

bool InfoCmd::OnCommand(const std::string &query, const std::string &port_data,
                        std::string *response_data) {
  std::vector<EndpointInfo> connections;
  RushConnection info;
  for(std::vector<endpoint_t>::iterator it = m_plugin.connection.begin();
      it != m_plugin.connection.end(); ++it) {
    if(0 == rush_connection_info(g_ctx, *it, &info)) {
      EndpointInfo conn_info;
      conn_info.handle = *it;
      conn_info.rtt = info.rtt;
      conn_info.address = info.address;
      conn_info.port = info.port;
      conn_info.upstream_bps = info.upstream_bps;
      conn_info.downstream_bps = info.downstream_bps;
      conn_info.packet_loss = info.packet_loss;
      conn_info.send_interval = info.send_interval;
      connections.push_back(conn_info);
    }
  }

  u16 upstream_limit_bps = 0;
  u16 downstream_limit_bps = 0;

  rush_limit_info(g_ctx, kRushUpstream, &upstream_limit_bps);
  rush_limit_info(g_ctx, kRushDownstream, &downstream_limit_bps);

  rush_time_t time = 0;
  rush_time(g_ctx, &time);

  Link::JsonWriter writer(*response_data);
  writer.Write("port", g_config.port);
  writer.Write("mtu", g_config.mtu);
  writer.Write("frame_time", m_plugin.IdleDt());
  writer.Write("time", time);
  writer.Write("connections", connections);
  writer.Write("upstream_limit_enabled", upstream_limit_bps != (u16)-1);
  writer.Write("upstream_limit_bps", upstream_limit_bps);
  writer.Write("downstream_limit_enabled", downstream_limit_bps != (16) - 1);
  writer.Write("downstream_limit_bps", downstream_limit_bps);
  writer.Finalize();
  return true;
}

UdpNode::UdpNode()
    : SimplePlugin(kUpdateDeltaMs), m_info(g_ctx, *this), m_connect(g_ctx),
      m_disconnect(g_ctx, *this), m_list(g_ctx), m_reg(g_ctx) {
  m_recv_buffer = malloc(kRecvBufferSize);
}

UdpNode::~UdpNode() { free(m_recv_buffer); }

bool UdpNode::OnStartup(const char *config, streamsize nbytes) {
  if(!RegisterCommand(&m_connect) || !RegisterCommand(&m_disconnect) ||
     !RegisterCommand(&m_list) || !RegisterCommand(&m_info) ||
     !RegisterCommand(&m_reg)) {
    return false;
  }

  RushCallbacks cbs = {&Link::OnStartup, &OnConnectivity, &Unpack,
                       &OutOfOrderUnpack, &Pack};

  g_config.port = 0;

  if(config && nbytes > 0) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.Parse(config, nbytes);

    if(err == tinyxml2::XML_SUCCESS) {
      tinyxml2::XMLElement *root = doc.RootElement();
      if(root->Attribute("port")) {
        g_config.port = root->IntAttribute("port");
      } else {
        PLUGIN_ERROR("no port specified");
        return false;
      }
    } else {
      PLUGIN_ERROR("problem parsing config: %s(%d)", doc.ErrorName(), err);
      return false;
    }
  } else {
    PLUGIN_ERROR("no config");
    return false;
  }

  g_config.port = 0;
  g_config.callbacks = cbs;
  g_config.data = static_cast<void *>(this);
  g_config.mtu = 256;

  g_ctx = rush_create(&g_config);
  if(!g_ctx) {
    PLUGIN_ERROR("problem creating rush context");
    return false;
  }
  PLUGIN_INFO("rush started at port %d", g_config.port);
  /*
          g_local = rush_open(g_ctx, Base::Url(Base::AddressIPv4::kLocalhost,
     g_port));
          if(g_local == 0) {
                  PLUGIN_ERROR("failed to open loopback connction");
                  rush_destroy(g_ctx);
                  return false;
          }
  */
  return true;
}

void UdpNode::OnShutdown() {
  rush_destroy(g_ctx);
  UnregisterCommand(&m_info);
  UnregisterCommand(&m_list);
  UnregisterCommand(&m_connect);
  UnregisterCommand(&m_disconnect);
  UnregisterCommand(&m_reg);
}

void UdpNode::OnUpdate(unsigned int dt) {
  if(g_ctx) {
    rush_update(g_ctx, dt);
  }
  rush_time_t time;

  for(std::vector<endpoint_t>::iterator it = connection.begin();
      it != connection.end(); ++it) {
    if(0 == rush_rtt(g_ctx, *it, &time)) {
      PLUGIN_INFO("%p rtt: %f. delta %d", *it, time, dt);
    }
  }
}

void UdpNode::OnRecvReady(const ConnectionNotification &notif) {
  SimplePlugin::Recv(notif.handle, m_recv_buffer, kRecvBufferSize,
                     [&](void *buffer, unsigned int nbytes) {
                       // ParseDataReceived(buffer, nbytes, notif.handle,
                       // notif.endpoint);
                     });
}

void UdpNode::OnNotification(const Notification &notif) {

  RestClient::ProcessNotification(notif);
}

} // namespace Link
