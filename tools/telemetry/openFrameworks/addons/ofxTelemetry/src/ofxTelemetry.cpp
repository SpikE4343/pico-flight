#include "ofxTelemetry.h"
#include "ofMain.h"
#include <string>
#include <vector>

//----------------------------------------------------------
ofxTelemetry::ofxTelemetry()
{
}

//----------------------------------------------------------
uint8_t telemetry_calc_crc(uint8_t *buf, uint8_t size)
{
  uint8_t crc = *buf;
  for (uint i = 1; i < size; ++i)
    crc ^= buf[i];

  return crc;
}

//----------------------------------------------------------
uint32_t ofxTelemetry::writeModPacket(const TDataValueMod_t& mod, ofBuffer &buf) const
{
  TDataModPacket_t packet;

  packet.header.marker = 0x7c;
  packet.header.type = PKT_DATA_MOD;
  packet.header.size = sizeof(packet.payload);
  packet.payload = mod;

  uint8_t *payload = (uint8_t *)&packet.payload;

  packet.crc = telemetry_calc_crc(payload, packet.header.size);
  buf.append((const char*)&packet, sizeof(packet));
  return sizeof(TDataModPacket_t);
}

//----------------------------------------------------------
void ofxTelemetry::update()
{
  if (!serial.isInitialized())
    return;

  uint8_t *data = NULL;
  uint32_t size = 0;

  if (sendBuffer.size() > 0)
    serial.writeBytes(sendBuffer.getData(), sendBuffer.size());

  sendBuffer.clear();

  uint32_t avail = serial.available();
  if (avail > 0)
  {
    readBuffer.clear();
    serial.readBytes(readBuffer, avail);
    // printf("avail: %u\n", avail);

    for (int i=0; i < avail; ++i)
    {
      // printf("rx.size: %u\n", recvBuffer.size());
      uint8_t d = (uint8_t)*(readBuffer.getData()+i);
      // printf("|%02x>>", d);
      recv(d);
    }
  }
}

//----------------------------------------------------------
uint32_t ofxTelemetry::count()
{
  return dataVars.size();
}



//----------------------------------------------------------
uint8_t *write_bytes(uint8_t *buf, uint8_t *data, int len)
{
  memcpy(buf, data, len);
  return buf + len;
}

//----------------------------------------------------------
uint8_t *write_string(uint8_t *buf, char *str)
{
  int len = strlen(str);
  assert(len < 256);

  uint8_t slen = (uint8_t)len;
  buf[0] = slen;

  memcpy(buf + 1, str, slen);
  return buf + slen + 1;
}

//----------------------------------------------------------
uint8_t *read_string(uint8_t *buf, std::string &str)
{
  uint8_t size = *buf++;
  str.assign((const char *)buf, size);
  return buf + size;
}

//----------------------------------------------------------
uint8_t *read_byte(uint8_t *buf, uint8_t *byte)
{
  *byte = *buf;
  return ++buf;
}

//----------------------------------------------------------
uint8_t *read_bytes(uint8_t *buf, uint8_t *dst, uint8_t size)
{
  memcpy(dst, buf, size);
  return buf + size;
}

void ofxTelemetry::onDataVarChange(DataVar& v)
{
  if(!v.sendChanges)
    return;

  TDataValueMod_t mod {
    .mod = Tdm_write,
    .time = 0,
    .value = {
      .id = v.id,
      .type = v.type,
      .value = v.v.get()
    }
  };

  writeModPacket(mod, sendBuffer);
}
//----------------------------------------------------------
void ofxTelemetry::processDataMod()
{
  DataVarMod msg;
  msg.read(recvBuffer);

  if (msg.mod.mod != Tdm_write)
    return;

  DataVar &var = dataVars[msg.mod.value.id];
  var.id = msg.mod.value.id;
  var.type = msg.mod.value.type;
  var.sendChanges = false;
  var.v.set(msg.mod.value.value);
  var.sendChanges = true;

  // float fval = var.getValue<float>();
  // var.v.setMin(fmin(var.param.getMin(), fval));
  // var.v.setMax(fmax(var.param.getMax(), fval));
  // var.v.set(fval);

  uint32_t time = ofGetSystemTimeMillis();
  auto key = DataVar::Sample::makeKey(time, msg.mod.time);
  DataVar::Sample &sample = var.samples[key];
  sample.v = var.v;
  sample.recvTime = time;
  sample.time = msg.mod.time;
}

//----------------------------------------------------------
void ofxTelemetry::processDataDesc()
{
  DataVar varMsg;
  varMsg.read(recvBuffer);

  DataVar &var = dataVars[varMsg.id];
  var.read(recvBuffer);
  var.sendChanges = true;

  if (var.v.getName().length() <= 0)
    var.v.setName(var.name);

  ofxNodeGui *hierarchy = var.isConfig() ? guiConfig : guiLive;
  ofxNodeGui *node = canvas;

  size_t pos = 0;
  size_t lastpos = 0;
  std::string token;
  ofxNodeGui *existing = NULL;
  ofxNodeGui *folder = NULL;
  bool layout = false;

  ofColor stripe =
      ofColor::fromHsb(
          var.id % 255,
          200, 220);

  ofColor sliderFill =
      ofColor::fromHsb(
          var.id % 255,
          150, 190);

  std::string indent = " ";
  while ((pos = var.name.find('.', lastpos)))
  {
    indent += " ";
    token = var.name.substr(lastpos, pos - lastpos);
    lastpos = pos + 1;

    existing = node ? node->getChildByName(token) : NULL;

    if(existing == NULL)
    {  
      // std::cout << token << std::endl;
      existing = node->getChildByName("_c");
      if(existing)
        existing = existing->getChildByName(token);
    }

    if (existing == NULL)
    {
      if (pos == std::string::npos)
      {
        existing = folder;
      }
      else
      {
        ofxNodeGuiStripeLabel* tl = new ofxNodeGuiStripeLabel(token+"_l", indent+token);
        tl->setColor(stripe);
        tl->setInteractable(true);
       
        ofxNodeGui* tc = new ofxNodeGui("_c");
        tc->setInteractable(true);
        tc->setLayout(new ofxNodeGuiVerticalLayout());
        tc->setVisible(false);

        ofxNodeGui* t = new ofxNodeGui(token);
        t->setDragTarget(tl);
        t->setInteractable(true);
        t->setDraggable(true);
        tl->setParent(t);
        tc->setParent(t);
        tc->setPosition(0, tl->getSize().y);

        
        float imgSize = tl->getSize().y/2;
        auto open = new ofxNodeGuiImage("_open");
        ofImage* imgOpen = new ofImage();
        imgOpen->load("images/icon-group-open.png");
        open->setImage(imgOpen);
        open->setSize(imgSize, imgSize);
        open->setVisible(false);
        open->setParent(t);
        open->setColor(stripe);

        auto closed = new ofxNodeGuiImage("_closed");
        ofImage* imgClosed = new ofImage();
        imgClosed->load("images/icon-group-closed.png");
        closed->setSize(imgSize, imgSize);
        closed->setImage(imgClosed);
        closed->setVisible(true);
        closed->setParent(t);
        closed->setColor(stripe);

        // ofPoint pos(tl->getSize().x - imgSize*2, tl->getSize().y-imgSize*1.5f);
        ofPoint pos(26.0f/2 * (indent.size()-1), tl->getSize().y-imgSize*1.5f);
        closed->setPosition(pos);
        open->setPosition(pos);

        tl->clicked.add(tc, &ofxNodeGui::toggleVisible, 0);
        tl->clicked.add((ofxNodeGui*)open, &ofxNodeGui::toggleVisible, 0 );
        tl->clicked.add((ofxNodeGui*)closed, &ofxNodeGui::toggleVisible, 0 );
        // tl->setPosition(26.0f/2 * (indent.size()-1), tl->getPosition().y);
        folder = node;
        existing = t;

        auto g = folder->getChildByName("_c");
        if(!g)
          g = folder;

        existing->setParent(g);
      }

      layout = true;
    }

    node = existing;
    folder = existing;

    if (pos == std::string::npos)
    {
      var.v.setName(token);
      break;
    }
  }

  if (var.guiFolder != folder)
  {
    var.guiFolder = folder;

    var.guiComp = new ofxNodeGui("gui_" + var.name);
    var.shortName = indent+token;

    var.onValueChange.add(this, &ofxTelemetry::onDataVarChange, 0);
    // var.guiComp->setValue(&var);
    
    var.guiComp->setParent(var.guiFolder->getChildByName("_c"));
    var.guiComp->setInteractable(true);

    auto tl = new ofxNodeGuiStripeLabel(token+"_l", indent+token);
    tl->setColor(stripe);
    tl->setInteractable(false);
    tl->setParent(var.guiComp);

    if(!var.isConfig() && var.isRealtime())
    {
      auto graph = new ofxNodeGuiDataVarGraph("graph_"+var.name);
      
      graph->setParent(var.guiComp);
      graph->setColor(stripe);
      graph->setPosition(270/2, 0);
      graph->setSize(270/2, 26);
      graph->setValue(&var);
    }

    {
      switch( var.type){
        case Tdt_b8:
        {
          auto toggle = new ofxNodeGuiToggle("tgl_"+var.name);
          
          toggle->setParent(var.guiComp);
          toggle->setColor(stripe);
          toggle->setPosition(270-26*1.5f, 0);
          toggle->setSize(26*0.75f, 26*0.75f);
          toggle->setValue(&var);
          toggle->setInteractable(true);
          toggle->clicked.add(toggle, &ofxNodeGuiToggle::toggleState, 0);
        }
        break;
        default:
        {
          auto num = new ofxNodeGuiNumeric("num_"+var.name);
          
          num->setParent(var.guiComp);
          num->setColor(stripe);
          num->setPosition(270/2, 0);
          num->setSize(270/2, 26);
          num->setValue(&var);
          num->setInteractable(true);
        }
        break;
      }
    }

    layout = true;

    existing->setColor(stripe);
  }

  if (layout)
  {
    hierarchy->applyLayout();
  }
}

//----------------------------------------------------------
void ofxTelemetry::recv(uint8_t byte)
{

  switch (recv_state)
  {
  case RECV_RESET:
    // printf("++++\n");
    memset(&header, 0, sizeof(header));
    recv_state = RECV_MARKER;
    crc = 0;
    offset = 0;
    recvBuffer.clear();
    recvBuffer.reserve(1024);
    recv(byte);

    break;

  case RECV_MARKER:
    // printf("%02x ", byte);
    if (byte == TELEMETRY_MARKER_BYTE)
    {
      header.marker = byte;
    }
    else if (header.marker == TELEMETRY_MARKER_BYTE)
    {
      header.type = byte;
      switch (header.type)
      {
      case PKT_DATA_MOD:
      case PKT_DATA_DESC_FRAME:
        recv_state = RECV_HEADER_SIZE;
        break;

      case PKT_NONE:
      default:
        recv_state = RECV_RESET;
        break;
      }
    }
    break;

  case RECV_HEADER_SIZE:
    // printf("%02x ", byte);
    header.size = byte;
    recv_state = header.size ? RECV_PAYLOAD : RECV_RESET;
    break;

  case RECV_PAYLOAD:
    // printf("%02x ", byte);
    if (recvBuffer.size() < header.size)
    {
      if (recvBuffer.size() == 0)
        crc = byte;
      else
        crc ^= byte;

      recvBuffer.append((const char *)&byte, 1);

      if (recvBuffer.size() == header.size)
      {
        recv_state = RECV_PAYLOAD_CRC;
      }
    }
    break;

  case RECV_PAYLOAD_CRC:
    // printf("%02x ", byte);
    if (crc == byte)
    {
      switch (header.type)
      {
      case PKT_DATA_MOD:
        processDataMod();
        break;

      case PKT_DATA_DESC_FRAME:
        processDataDesc();
        break;

      case PKT_NONE:
      default:
        break;
      }
    }
    else
    {
      printf("crc invalid: %u != %u\n", crc, byte);
    }

    recv_state = RECV_RESET;
    // printf("----\n");
    break;
  }
}


//----------------------------------------------------------
// Data Variable
uint32_t ofxTelemetry::DataVar::read(ofBuffer &buffer)
{
  uint8_t *buf = (uint8_t *)buffer.getData();
  buf = read_bytes(buf, (uint8_t *)&id, sizeof(id));
  buf = read_bytes(buf, (uint8_t *)&type, sizeof(type));
  buf = read_bytes(buf, (uint8_t *)&modsAllowed, sizeof(modsAllowed));
  buf = read_string(buf, name);
  buf = read_string(buf, desc);
  return (uint8_t *)buffer.getData() - buf;
}

//----------------------------------------------------------
uint8_t ofxTelemetry::DataVar::write(ofBuffer &buf) const
{
  int start = buf.size();
  buf.append((const char *)&id, sizeof(id));
  buf.append((const char *)&type, sizeof(type));
  buf.append((const char *)&modsAllowed, sizeof(modsAllowed));
  uint8_t strlen = name.length();
  buf.append((const char *)&strlen, sizeof(strlen));
  buf.append(name);

  strlen = desc.length();
  buf.append((const char *)&strlen, sizeof(strlen));
  buf.append(desc);

  return buf.size() - start;
}

//----------------------------------------------------------
template<>
std::string ofxTelemetry::DataVar::getValue() const
{
  switch (type)
  {
  case Tdt_f32:
    return ofToString(v.get().f32);

  case Tdt_u8:
  case Tdt_u32:
  case Tdt_u16:
    return ofToString(v.get().u32);

  case Tdt_i8:
  case Tdt_i32:
  case Tdt_i16:
    return ofToString(v.get().i32);

  case Tdt_b8:
    return ofToString(v.get().b8);
  case Tdt_c8:
    return ofToString(v.get().c8);
  }

  return "{unknown:" + ofToString(type) + "}";
}

//----------------------------------------------------------
template< typename TValueType>
TValueType ofxTelemetry::DataVar::getValue() const
{
  switch (type)
  {
  case Tdt_f32:
    return (TValueType)v.get().f32;

  case Tdt_u32:
    return (TValueType)v.get().u32;
  case Tdt_i32:
    return (TValueType)(v.get().i32);

  case Tdt_u16:
    return (TValueType)(v.get().u16);
  case Tdt_i16:
    return (TValueType)(v.get().i16);

  case Tdt_u8:
    return (TValueType)(v.get().u8);
  case Tdt_i8:
    return (TValueType)(v.get().i8);

  case Tdt_b8:
    return (TValueType)(v.get().b8);
  case Tdt_c8:
    return (TValueType)(v.get().c8);
  }

  return 0.0f;
}

//----------------------------------------------------------
template< typename TValueType>
void ofxTelemetry::DataVar::setValue(TValueType fv, bool notify)
{
  TValue_t vt = v.get();
  switch (type)
  {
  case Tdt_f32:
    vt.f32 = (float)fv;
    break;

  case Tdt_u32:
    vt.u32 = (uint32_t)fv;
    break;

  case Tdt_i32:
    vt.i32 = (int32_t)fv;
    break;

  case Tdt_u16:
    vt.u16 = (uint16_t)fv;
    break;

  case Tdt_i16:
    vt.i16 = (int16_t)fv;
    break;

  case Tdt_u8:
    vt.u8  = (uint8_t)fv;
    break;

  case Tdt_i8:
    vt.i8  = (int8_t)fv;
    break;  

  case Tdt_b8:
    vt.b8 = fv ? true : false;
    break;

  case Tdt_c8:
    vt.c8 = (char)fv;
    break;
  }

  v.set(vt);
  if(notify)
    onValueChange.notify(*this);
}

//----------------------------------------------------------
template<>
void ofxTelemetry::DataVar::setValue(std::string fv, bool notify)
{
  TValue_t vt = v.get();
  switch (type)
  {
  case Tdt_f32:
    vt.f32 = ofFromString<float>(fv);
    break;

  case Tdt_u32:
  case Tdt_u16:
  case Tdt_u8:
    vt.u32 = ofFromString<uint32_t>(fv);
    break;

  case Tdt_i32:
  case Tdt_i16:
  case Tdt_i8:
    vt.i32 = ofFromString<int32_t>(fv);
    break;

  case Tdt_b8:
    vt.b8 = ofFromString<bool>(fv);
    break;

  case Tdt_c8:
    vt.c8 = ofFromString<char>(fv);
    break;
  }

  v.set(vt);
  if(notify)
    onValueChange.notify(*this);
}

void ofxTelemetry::DataVar::increment(float value, bool notify)
{
  TValue_t vt;

  switch(type)
  {
    case Tdt_f32:
      vt.f32 = v.get().f32 + value;
      break;

    case Tdt_u32:
    case Tdt_u16:
    case Tdt_u8:
    case Tdt_b8:
    case Tdt_c8:
      vt.u32 = v.get().u32 + (uint32_t)value; 
      break;

    case Tdt_i32:
    case Tdt_i16:
    case Tdt_i8:
      vt.i32 = v.get().i32 + (int32_t)value; 
      break;
  }

  v.set(vt);
  if(notify)
    onValueChange.notify(*this);
}

//----------------------------------------------------------
template<>
std::string ofxTelemetry::DataVar::Sample::getValue(uint8_t type) const
{
  switch (type)
  {
  case Tdt_f32:
    return ofToString(v.f32);

  case Tdt_u32:
    return ofToString(v.u32);
  case Tdt_i32:
    return ofToString(v.i32);

  case Tdt_u16:
    return ofToString(v.u16);
  case Tdt_i16:
    return ofToString(v.i16);

  case Tdt_u8:
    return ofToString(v.u8);
  case Tdt_i8:
    return ofToString(v.i8);

  case Tdt_b8:
    return ofToString(v.b8);
  case Tdt_c8:
    return ofToString(v.c8);
  }

  return "{unknown:" + ofToString(type) + "}";
}

//----------------------------------------------------------
template< typename TValueType>
TValueType ofxTelemetry::DataVar::Sample::getValue(uint8_t type) const
{
  switch (type)
  {
  case Tdt_f32:
    return (TValueType)v.f32;

  case Tdt_u32:
    return (TValueType)v.u32;
  case Tdt_i32:
    return (TValueType)(v.i32);

  case Tdt_u16:
    return (TValueType)(v.u16);
  case Tdt_i16:
    return (TValueType)(v.i16);

  case Tdt_u8:
    return (TValueType)(v.u8);
  case Tdt_i8:
    return (TValueType)(v.i8);

  case Tdt_b8:
    return (TValueType)(v.b8);
  case Tdt_c8:
    return (TValueType)(v.c8);
  }

  return 0.0f;
}

//----------------------------------------------------------
template< typename TValueType>
void ofxTelemetry::DataVar::Sample::setValue(uint8_t type, TValueType fv)
{
  switch (type)
  {
  case Tdt_f32:
    v.f32 = (float)fv;
    break;

  case Tdt_u32:
    v.u32 = (uint32_t)fv;
    break;

  case Tdt_i32:
    v.i32 = (int32_t)fv;
    break;

  case Tdt_u16:
    v.u16 = (uint16_t)fv;
    break;

  case Tdt_i16:
    v.i16 = (int16_t)fv;
    break;

  case Tdt_u8:
    v.u8  = (uint8_t)fv;
    break;

  case Tdt_i8:
    v.i8  = (int8_t)fv;
    break;  

  case Tdt_b8:
    v.b8 = (bool)fv;
    break;

  case Tdt_c8:
    v.c8 = (char)fv;
    break;
  }
}

template<>
void ofxTelemetry::DataVar::Sample::setValue(uint8_t type, const std::string& fv)
{
  switch (type)
  {
  case Tdt_f32:
    v.f32 = ofFromString<float>(fv);
    break;

  case Tdt_u32:
    v.u32 = ofFromString<uint32_t>(fv);
    break;

  case Tdt_i32:
    v.i32 = ofFromString<int32_t>(fv);
    break;

  case Tdt_u16:
    v.u16 = ofFromString<uint16_t>(fv);
    break;

  case Tdt_i16:
    v.i16 = ofFromString<int16_t>(fv);
    break;

  case Tdt_u8:
    v.u8  = ofFromString<uint8_t>(fv);
    break;

  case Tdt_i8:
    v.i8  = ofFromString<int8_t>(fv);
    break;  

  case Tdt_b8:
    v.b8 = ofFromString<bool>(fv);
    break;

  case Tdt_c8:
    v.c8 = ofFromString<char>(fv);
    break;
  }
}




//----------------------------------------------------------
uint32_t ofxTelemetry::DataVarMod::read(ofBuffer &buffer)
{
  uint8_t *buf = (uint8_t *)buffer.getData();
  buf = read_bytes(buf, (uint8_t *)&mod, sizeof(TDataValueMod_t));
  return (uint8_t *)buffer.getData() - buf;
}

//----------------------------------------------------------
uint8_t ofxTelemetry::DataVarMod::write(ofBuffer &buf) const
{
  int start = buf.size();
  buf.append((const char *)&mod, sizeof(TDataValueMod_t));
  return buf.size() - start;
}




// GUI

//----------------------------------------------------------
bool isPointInRect(const ofRectangle& r, const ofPoint& p)
{
  	return p.x >= r.getMinX() && p.y >= r.getMinY() &&
		   p.x <= r.getMaxX() && p.y <= r.getMaxY();
}

//----------------------------------------------------------
ofRectangle ofxNodeGuiVerticalLayout::apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children)
{
  ofRectangle size(0, 0, 0, 0);

  auto padding = parent.getStyle() ? parent.getStyle()->padding : ofPoint::zero();
  auto offset = padding;

  for (uint i = 0; i < children.size(); i++)
  {
    children[i]->setPosition(offset);
    children[i]->applyLayout();
    ofPoint childSize = children[i]->getSize();
    offset.y += childSize.y + ((i == children.size() - 1) ? 0 : padding.y);

    size.growToInclude((offset) + ofPoint(childSize.x, 0));
  }

  return size;
}

//----------------------------------------------------------
ofRectangle ofxNodeGuiHorizontalLayout::apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children)
{
  ofRectangle size(0, 0, 0, 0);

  auto padding = parent.getStyle() ? parent.getStyle()->padding : ofPoint::zero();
  auto offset = padding;

  for (uint i = 0; i < children.size(); i++)
  {
    children[i]->setPosition(offset);
    children[i]->applyLayout();
    ofPoint childSize = children[i]->getSize();
    offset.x += childSize.x + ((i == children.size() - 1) ? 0 : padding.x);

    size.growToInclude((offset) + ofPoint(childSize.y, 0));
  }

  return size;
}

//----------------------------------------------------------
ofRectangle ofxNodeGuiNestedLayout::apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children)
{
  ofRectangle size(0, 0, 0, 0);

  auto padding = parent.getStyle() ? parent.getStyle()->padding : ofPoint::zero();
  ofPoint offset = padding;
  offset.x += parent.getStyle()->indent;

  for (uint i = 0; i < children.size(); i++)
  {
    children[i]->setPosition(offset);
    children[i]->applyLayout();
    ofPoint childSize = children[i]->getSize();

    offset.y += childSize.y + padding.y;
    size.growToInclude((offset) + ofPoint(childSize.x, 0));
  }

  return size;
}

//----------------------------------------------------------
ofRectangle ofxNodeParentSizeLayout::apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children)
{
  parent.setSize(parent.getParent()->getSize() * sizeScalar);  
  for (uint i = 0; i < children.size(); i++)
  {
    children[i]->applyLayout();
  }

  return ofRectangle(); //parent.getSize();
}


//----------------------------------------------------------
// Node
ofxNodeGui::ofxNodeGui()
    : style(NULL), layout(NULL), visible(true), interactable(false), draggable(false), hover(false), focus(false), parent(NULL), dragTarget(NULL), position(0, 0, 0)
{
}

//----------------------------------------------------------
ofxNodeGui::ofxNodeGui(const std::string &name)
    : style(NULL), layout(NULL), visible(true), interactable(false), draggable(false), hover(false), focus(false), parent(NULL), dragTarget(NULL), position(0, 0, 0), name(name)
{
}

//----------------------------------------------------------
ofxNodeGui::~ofxNodeGui()
{
}

//----------------------------------------------------------
void ofxNodeGui::applyLayout()
{
  ofRectangle rect;

  if(visible)
  {
    if (layout)
    {
      rect = layout->apply(*this, children);
    }
    else
    {
      for (uint i = 0; i < children.size(); i++)
      {
        // cout << "   " << name << "." << children[i]->getName() << endl;
        children[i]->applyLayout();
        rect.growToInclude(children[i]->getPosition() + children[i]->getSize());
        // cout << rect << endl;
      }
    }
  }

  size.set(rect.getWidth(), rect.getHeight(), 0);
}

//----------------------------------------------------------
void ofxNodeGui::setStyle(ofxNodeGuiStyle *style, bool setChildren)
{
  this->style = style;

  if (setChildren)
  {
    for (uint i = 0; i < children.size(); i++)
      children[i]->setStyle(style, setChildren);
  }
}

//----------------------------------------------------------
void ofxNodeGui::setColor(const ofColor &s, bool setChildren)
{
  this->stripeColor = s;

  if (setChildren)
  {
    for (uint i = 0; i < children.size(); i++)
      children[i]->setColor(s, setChildren);
  }
}

//----------------------------------------------------------
void ofxNodeGui::update()
{
}

//----------------------------------------------------------
void ofxNodeGui::draw()
{
  if (!visible)
    return;

  ofPushMatrix();
  ofTranslate(position);

  if (hover)
  {
    ofPushStyle();
    ofSetColor(stripeColor, 50);
    ofDrawRectangle(0, 0, size.x, size.y);
    ofPopStyle();
  }

  for (uint i = 0; i < children.size(); i++)
    children[i]->draw();

  ofPopMatrix();
}

//----------------------------------------------------------
ofxNodeGui *ofxNodeGui::hitTest(ofPoint point)
{
  static int level = 0;

  if(!visible)
    return NULL;

  ofPoint pos = getGlobalPosition();
  ofRectangle rect(pos, size.x, size.y);
  if (!isPointInRect(rect,point))
    return NULL;

  for (int i = (int)children.size() - 1; i >= 0; --i)
  {
    ++level;
    auto hit = children[i]->hitTest(point);
    --level;

    // for (int l = 0; l < level; ++l)
    //   cout << " ";

    // cout << name << " [" << i << "]"
    //      << children[i]->getName() << ", in:"
    //      << children[i]->isInteractable() << " - hit: "
    //      << (hit ? hit->getName() : "null") << endl;

    if (hit == NULL)
      continue;

    if (dragTarget != NULL)
    {
      if (hit == dragTarget)
        return this;

      if (!hit->isInteractable())
        continue;
    }

    if (!hit->isInteractable())
      continue;

    return hit->isInteractable() ? hit : this;
  }
  return this;
}

//----------------------------------------------------------
void ofxNodeGui::attach(ofxNodeGui *child)
{
  if (child == NULL)
    return;

  if (child->getStyle() == NULL)
    child->setStyle(style);

  children.push_back(child);
}

//----------------------------------------------------------
ofxNodeGui *ofxNodeGui::getChildByName(const std::string &n) const
{
  for (auto it = children.begin(); it != children.end(); ++it)
  {
    ofxNodeGui *node = *it;
    if (node->getName() == n)
      return node;
  }

  return NULL;
}

//----------------------------------------------------------
void ofxNodeGui::onHover(bool hover)
{
  this->hover = hover;
}

//----------------------------------------------------------
void ofxNodeGui::onPress(int x, int y, int button)
{
}

//----------------------------------------------------------
void ofxNodeGui::onRelease(int x, int y, int button)
{
}

//----------------------------------------------------------
void ofxNodeGui::keyPressed(int key)
{

}

//----------------------------------------------------------
void ofxNodeGui::keyReleased(int key)
{

}


//--------------------------------------------------------------
// Canvas
ofxNodeGuiCanvas::ofxNodeGuiCanvas(const std::string& name)
  : focusedNode(NULL)
  , lastHitNode(NULL)
  , lastHitMousePos(0,0)
  , scroll(0,0)
  , lastHitDrag(false)
  {

  }

//--------------------------------------------------------------
void ofxNodeGuiCanvas::keyPressed(int key)
{
  if(focusedNode)
    focusedNode->keyPressed(key);
}

//--------------------------------------------------------------
void ofxNodeGuiCanvas::keyReleased(int key)
{
  if(focusedNode)
    focusedNode->keyReleased(key);
}

//--------------------------------------------------------------
void ofxNodeGuiCanvas::mouseMoved(int x, int y)
{
  lastHitMousePos.set(x, y, 0);
  auto hit = hitTest(lastHitMousePos - scroll);

  if (lastHitNode != hit)
  {
    if (lastHitNode != NULL)
    {
      lastHitNode->onHover(false);
      auto dt = lastHitNode->getDragTarget();
      if(dt) 
        dt->onHover(false);
      cout << lastHitNode->getName() << " - unhover" << endl;
    }

    lastHitNode = hit;

    if (lastHitNode != NULL)
    {
      lastHitNode->onHover(true);
      auto dt = lastHitNode->getDragTarget();
      if(dt) 
        dt->onHover(true);
      cout << lastHitNode->getName() << " + hover" << endl;
    }
  }
}

//--------------------------------------------------------------
void ofxNodeGuiCanvas::mouseDragged(int x, int y, int button)
{
  ofPoint mpos(x, y);
  ofPoint delta = mpos - lastHitMousePos;
  lastHitMousePos = mpos;

  if( delta.length() < 2)
  {
    lastHitDrag = false;
    return;
  }

  lastHitDrag = true;

  if (lastHitNode == NULL || lastHitNode == this)
  {
    scroll += delta;
    return;
  }

  if (!lastHitNode->isDraggable())
    return;

  if (lastHitNode == this)
    return;

  ofPoint pos = lastHitNode->getPosition() + delta;
  lastHitNode->setPosition(pos);
}

//--------------------------------------------------------------
void ofxNodeGuiCanvas::mousePressed(int x, int y, int button)
{
  if(focusedNode && focusedNode != lastHitNode)
  {
    focusedNode->onFocus(false);
    cout << "Lost Focus:" << button << " on " << focusedNode->getName() << endl;
    focusedNode = NULL;
  }

  if (lastHitNode)
  {
    if(lastHitNode->canFocus())
    {
      focusedNode = lastHitNode;
      lastHitNode->onFocus(true);
      cout << "Gained Focus:" << button << " on " << focusedNode->getName() << endl;
    }
    else
    {
      focusedNode = NULL;
      
    }

    if(lastHitNode->isInteractable())
    {
      cout << "Press:" << button << " on " << lastHitNode->getName() << endl;
      lastHitNode->onPress(x, y, button);
    }
  }
}

//--------------------------------------------------------------
void ofxNodeGuiCanvas::mouseReleased(int x, int y, int button)
{
  if (lastHitNode && lastHitNode->isInteractable() && (!lastHitNode->isDraggable() || !lastHitDrag))
  {
    cout << "Release:" << button << " on " << lastHitNode->getName() << endl;
    lastHitNode->onRelease(x, y, button);

    lastHitNode->clicked.notify();

    auto dt = lastHitNode->getDragTarget();
    if(dt)
      dt->clicked.notify();
  }


  lastHitDrag = false;
}

//--------------------------------------------------------------
void ofxNodeGuiCanvas::mouseScrolled(int x, int y, float scrollX, float scrollY)
{
  ofPoint offset(scrollX, scrollY);

  offset *= 10.0f;
  scroll += offset;
}

//--------------------------------------------------------------
void ofxNodeGuiCanvas::draw()
{
  ofPushMatrix();
    ofTranslate(scroll);
    applyLayout();
    ofxNodeGui::draw();
  ofPopMatrix();
}


//----------------------------------------------------------
// Label
ofxNodeGuiLabel::ofxNodeGuiLabel(const std::string &name, const std::string &text)
    : ofxNodeGui(name), text(text)
{
  this->setSize(270, 26);
}

//----------------------------------------------------------
ofxNodeGuiLabel::ofxNodeGuiLabel(const std::string &text)
    : ofxNodeGui(text), text(text)
{
  this->setSize(270, 26);
}

//----------------------------------------------------------
void ofxNodeGuiLabel::applyLayout()
{
  // auto bounds = style->font.getStringBoundingBox(text, 0, 0);
  // size.set(bounds.getWidth(), bounds.getHeight());
}

//----------------------------------------------------------
void ofxNodeGuiLabel::update()
{
}

//----------------------------------------------------------
void ofxNodeGuiLabel::draw()
{
  if (!visible)
    return;

  ofPushMatrix();
  ofTranslate(position);

  ofFill();

  if (hover || focus)
    ofSetColor(style->hover);
  else
    ofSetColor(style->background);

  ofDrawRectangle(0, 0, size.x, size.y);

  ofSetColor(style->text);

  auto bounds = style->font.getStringBoundingBox(text, 0, 0);

  // left justify and center alight text for now
  style->font.drawString(
      text,
      style->stripeWidth + style->padding.x + 12,
      bounds.getHeight() - bounds.getHeight() / 2 + size.y / 2);
  ofPopMatrix();
}

//----------------------------------------------------------
ofxNodeGui *ofxNodeGuiLabel::hitTest(ofPoint point)
{
  return ofxNodeGui::hitTest(point);
}


//----------------------------------------------------------
// Stripe Label - TODO: remove
ofxNodeGuiStripeLabel::ofxNodeGuiStripeLabel(const std::string &name, const std::string &text)
    : ofxNodeGuiLabel(name, text)
{
}

//----------------------------------------------------------
ofxNodeGuiStripeLabel::ofxNodeGuiStripeLabel(const std::string &text)
    : ofxNodeGuiLabel(text)
{
}

//----------------------------------------------------------
void ofxNodeGuiStripeLabel::draw()
{
  if (!visible)
    return;

  ofPushMatrix();
  ofTranslate(position);

  ofFill();
  if (hover || focus)
    ofSetColor(style->hover);
  else
    ofSetColor(style->background);
  ofDrawRectangle(0, 0, size.x, size.y);

  ofSetColor(stripeColor);
  ofDrawRectangle(0, 0, style->stripeWidth, size.y);
  ofSetColor(style->text);

  auto bounds = style->font.getStringBoundingBox(text, 0, 0);

  // left justify and center alight text for now
  style->font.drawString(
      text,
      style->stripeWidth + style->padding.x + 12,
      bounds.getHeight() - bounds.getHeight() / 2 + size.y / 2);

  ofPopMatrix();
}


//----------------------------------------------------------
// Text Field
ofxNodeGuiTextField::ofxNodeGuiTextField(const std::string &name, const std::string &text)
    : ofxNodeGuiLabel(name, text)
{
  this->setSize(270, 26);
}

//----------------------------------------------------------
ofxNodeGuiTextField::ofxNodeGuiTextField(const std::string &text)
    : ofxNodeGuiLabel(text, text)
{
  this->setSize(270, 26);
}



//----------------------------------------------------------
// Graph
ofxNodeGuiDataVarGraph::ofxNodeGuiDataVarGraph(const std::string &name)
    : ofxNodeGuiDataVar(name)
    , linePoints(128)
{
  lineMesh.setMode(OF_PRIMITIVE_LINE_STRIP);
  lineMesh.enableColors();
  lineMesh.enableIndices();

  lineMesh.getVertices().resize(linePoints);
  lineMesh.getColors().resize(linePoints);
  lineMesh.getIndices().resize(linePoints);

  for(uint i=0; i < linePoints; ++i)
  {
    lineMesh.setVertex(i, ofPoint(0, 0, 0) );
    lineMesh.setIndex(i,i);
  }
}

//----------------------------------------------------------
void ofxNodeGuiDataVarGraph::update()
{
  
}

//----------------------------------------------------------
void ofxNodeGuiDataVarGraph::setValue(ofxTelemetry::DataVar *v)
{
  if (value)
    value->v.removeListener(this, &ofxNodeGuiDataVarGraph::onValueChange);

  value = v;

  if(value)
    value->v.addListener(this, &ofxNodeGuiDataVarGraph::onValueChange);

  update();
}

//----------------------------------------------------------
void ofxNodeGuiDataVarGraph::onValueChange(TValue_t &v)
{
  update();
}

//----------------------------------------------------------
void ofxNodeGuiDataVarGraph::applyLayout()
{
  // auto bounds = style->font.getStringBoundingBox(text, 0, 0);
  // size.set(bounds.getWidth(), bounds.getHeight());
}

//----------------------------------------------------------
void ofxNodeGuiDataVarGraph::draw()
{
  if (!visible || !value)
    return;

  ofPushMatrix();
    ofTranslate(position + ofPoint(0, size.y));

    ofSetColor(stripeColor);


    float maxY = -100000000.0f;
    float minY = 1000000000.f;
    int maxSamples = linePoints;
    lineStep = size.x / (float)linePoints;

    for( auto samp = value->samples.rbegin(); maxSamples > 0 && samp != value->samples.rend(); ++samp, --maxSamples)
    {
      maxY = fmax(maxY, samp->second.getValue<float>(value->type));
      minY = fmin(minY, samp->second.getValue<float>(value->type));
    }

    maxSamples = linePoints;
    float range = size.y/2;
    ofPoint vert(0,0,0);
    ofColor c = stripeColor;
    float yrange = maxY - minY;
    auto samp = value->samples.rbegin();
    for( ;maxSamples >= 0; --maxSamples)
    {
      float v = 0.0f;
      if(samp != value->samples.rend())
      {
        // vert.y = ofMap(v, minY, maxY, range, -range, true)-range;
        v = samp->second.getValue<float>(value->type);
        vert.y = ofMap(v, minY, maxY, range, -range, true)-range;
        c.setSaturation(ofMap(v*v, 0, yrange, 100, 255, true));
        ++samp;
      }
      else
      {
        v = maxY-minY;
        vert.y = ofMap(v, minY, maxY, range, -range, true)-range;
        c.setSaturation(ofMap(fabsf(v), 0, range, 100, 255, true));
      }

      lineMesh.setVertex(maxSamples, vert);
      lineMesh.setColor(maxSamples, c );
      vert.x+=lineStep;
    }

    lineMesh.draw();
    
  ofPopMatrix();
}


//----------------------------------------------------------
// Image
ofxNodeGuiImage::ofxNodeGuiImage(const std::string &name)
  : ofxNodeGui(name)
  , image(nullptr)
{

}

//----------------------------------------------------------
void ofxNodeGuiImage::setImage(ofImage* i, bool useSize) 
{ 
  image = i;
  if(useSize && image)
    size.set(image->getWidth(), image->getHeight());
}

//----------------------------------------------------------
void ofxNodeGuiImage::draw()
{
  if(!visible || !image)
    return;

  ofPushMatrix();
    ofTranslate(position);
    ofPushStyle();

    ofSetColor(stripeColor);
    image->draw(0,0,size.x, size.y);
    ofPopStyle();
  ofPopMatrix();
}

//----------------------------------------------------------
// ofxNodeGuiDataVar
ofxNodeGuiDataVar::ofxNodeGuiDataVar(const std::string &name)
  : ofxNodeGui(name)
  , value(NULL)
{

}


//----------------------------------------------------------
void ofxNodeGuiDataVar::setValue(ofxTelemetry::DataVar *v)
{
  value = v;
}


//----------------------------------------------------------
// ofxNodeGuiToggle
ofxNodeGuiToggle::ofxNodeGuiToggle(const std::string &name)
  : ofxNodeGuiDataVar(name)
{

}


//----------------------------------------------------------
void ofxNodeGuiToggle::draw()
{
  if(!visible)
    return;

  ofPushMatrix();
    ofTranslate(position);
    ofPushStyle();
    if(hover)
      ofSetColor(style->hover);
    else
      ofSetColor(stripeColor);
    
    ofNoFill();

    ofDrawCircle(size.x/2, size.y/2, (size.y/2)*0.75f);
    if(value && value->getValue<bool>())
    {
      ofFill();
      ofDrawCircle(size.x/2, size.y/2, (size.y/2)*0.5f);
      // ofDrawRectangle(0,0,size.x,size.y);
    }
    ofPopStyle();
  ofPopMatrix();
}



//----------------------------------------------------------
// ofxNodeGuiToggle
ofxNodeGuiNumeric::ofxNodeGuiNumeric(const std::string &name)
  : ofxNodeGuiDataVar(name)
  , cursorIndex(-1)
  , editing(false)
{

}


//----------------------------------------------------------
void ofxNodeGuiNumeric::draw()
{
  if (!visible)
    return;

  ofPushMatrix();
  ofTranslate(position);

  auto text = value->getValue<std::string>();

  ofFill();
  if (canFocus() && (hover || focus))
  {
    if(editing)
      ofSetColor(stripeColor, 128);
    else
      ofSetColor(style->hover, 128);

    ofDrawRectangle(0, 0, size.x, size.y);
  }

  // ofSetColor(stripeColor);
  // ofDrawRectangle(0, 0, style->stripeWidth, size.y);
  ofSetColor(style->text);

  if(editing)
  {
    text = editingText;
  }

  auto bounds = style->font.getStringBoundingBox(text, 0, 0);

  // left justify and center alight text for now
  style->font.drawString(
      text,
      // style->stripeWidth + style->padding.x + 12,
      size.x - (bounds.getWidth() + size.y/2 + style->padding.x),
      bounds.getHeight() - bounds.getHeight() / 2 + size.y / 2);

  ofPopMatrix();
}

void ofxNodeGuiNumeric::keyPressed(int key)
{

  if(focus)
  {
    switch(key)
    {
      // TODO: define increment per value?
      case OF_KEY_UP:
        value->increment(value->isFloat() ? 0.01f : 1.0f);
        break;

      case OF_KEY_DOWN:
        value->increment(value->isFloat() ? -0.01f : -1.0f);
        break;
    }
  }

  if(!editing)
    return;


  switch (key)
  {
  case OF_KEY_ESC:
    editing = false;
    break;
  
  case OF_KEY_BACKSPACE:
    editingText = editingText.substr(0, editingText.size()-1);
    break;

  case OF_KEY_DEL:
    editingText = "";
    break;

  case OF_KEY_RETURN:
    value->setValue(editingText, true);
    editing = false;
    editingText = "";
    break;

  default:
    if (key>=48 && key<=57)
    {
      editingText += (char)key;
    }
    else if(key == '.')
    {
      if(value->isFloat())
        editingText += (char)key;
    }
    else if( key == '-')
    {
      switch(value->type)
      {
        case Tdt_u8:
        case Tdt_u16:
        case Tdt_u32:
          break;

        default:
          editingText += (char)key;
          break;
      }
    }
    break;
  }
    
}

void ofxNodeGuiNumeric::keyReleased(int key)
{

}

void ofxNodeGuiNumeric::onFocus(bool f)
{
  ofxNodeGuiDataVar::onFocus(f);

  if(editing)
  {
    // actively editing
    if(!f)
    {
      // lost focus to apply changes?
      value->setValue(editingText, true);
      editing = false;
    }
  }
  else
  {
    if(f)
    {
      editingText = value->getValue<std::string>();
      editing = true;
    }
    else
    {

    }
    // not editiing
  }
}