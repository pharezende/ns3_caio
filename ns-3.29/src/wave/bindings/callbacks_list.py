callback_classes = [
    ['ns3::ObjectBase *', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['bool', 'ns3::Ptr<ns3::NetDevice>', 'ns3::Ptr<const ns3::Packet>', 'unsigned short', 'const ns3::Address &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['bool', 'ns3::Ptr<ns3::NetDevice>', 'ns3::Ptr<const ns3::Packet>', 'unsigned short', 'const ns3::Address &', 'const ns3::Address &', 'ns3::NetDevice::PacketType', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::NetDevice>', 'ns3::Ptr<const ns3::Packet>', 'unsigned short', 'const ns3::Address &', 'const ns3::Address &', 'ns3::NetDevice::PacketType', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::NetDevice>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::Socket>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['bool', 'ns3::Ptr<ns3::Socket>', 'const ns3::Address &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::Socket>', 'const ns3::Address &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::Socket>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'const ns3::Ipv4Header &', 'ns3::Ptr<const ns3::Packet>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::Packet>', 'ns3::Ptr<ns3::Ipv4>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'const ns3::Ipv4Header &', 'ns3::Ptr<const ns3::Packet>', 'ns3::Ipv4L3Protocol::DropReason', 'ns3::Ptr<ns3::Ipv4>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::Packet>', 'ns3::Ptr<ns3::Ipv6>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'const ns3::Ipv6Header &', 'ns3::Ptr<const ns3::Packet>', 'ns3::Ipv6L3Protocol::DropReason', 'ns3::Ptr<ns3::Ipv6>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'const ns3::Ipv6Header &', 'ns3::Ptr<const ns3::Packet>', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::Packet>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'std::basic_string<char>', 'ns3::Ptr<const ns3::Packet>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Mac48Address', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::Packet>', 'ns3::Mac48Address', 'ns3::Mac48Address', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Mac48Address', 'unsigned char', 'bool', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Mac48Address', 'unsigned char', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'const ns3::WifiMacHeader &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'unsigned int', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Time', 'ns3::Time', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::WifiMacQueueItem>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['bool', 'ns3::Ptr<ns3::WifiMac>', 'const ns3::OrganizationIdentifier &', 'ns3::Ptr<const ns3::Packet>', 'const ns3::Address &', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['bool', 'ns3::Ptr<const ns3::Packet>', 'const ns3::Address &', 'unsigned int', 'unsigned int', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Address', 'ns3::Address', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::MobilityModel>', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::Packet>', 'double', 'ns3::WifiTxVector', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::Packet>', 'double', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Time', 'ns3::Time', 'WifiPhyState', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::Packet>', 'double', 'ns3::WifiMode', 'ns3::WifiPreamble', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::Packet>', 'double', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::Packet>', 'ns3::WifiMode', 'ns3::WifiPreamble', 'unsigned char', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::Packet>', 'unsigned short', 'ns3::WifiTxVector', 'ns3::MpduInfo', 'ns3::SignalNoiseDbm', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<const ns3::Packet>', 'unsigned short', 'ns3::WifiTxVector', 'ns3::MpduInfo', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
    ['void', 'ns3::Ptr<ns3::Packet>', 'const ns3::WifiMacHeader *', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty', 'ns3::empty'],
]
