#pragma once

#include <map>
#include <vector>

#include "ofSerial.h"
#include "ofFileUtils.h"
#include "ofParameter.h"
#include "ofTrueTypeFont.h"

extern "C"
{
#include "telemetry_data.h"
}

class ofxNodeGui;

//----------------------------------------------------------
class ofxNodeGuiStyle
{
public:
  ofColor stripe;
  ofColor background;
  ofColor text;
  ofColor hover;

  ofPoint padding;
  ofPoint labelSize;
  float indent;

  ofTrueTypeFont font;
  float stripeWidth;
};

//----------------------------------------------------------
class ofxNodeGuiLayout
{
public:
  virtual ofRectangle apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children) = 0;
};

//----------------------------------------------------------
class ofxNodeGuiVerticalLayout : public ofxNodeGuiLayout
{
public:
  virtual ofRectangle apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children);
};

//----------------------------------------------------------
class ofxNodeGuiHorizontalLayout : public ofxNodeGuiLayout
{
public:
  virtual ofRectangle apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children);
};

//----------------------------------------------------------
class ofxNodeGuiNestedLayout : public ofxNodeGuiLayout
{
public:
  virtual ofRectangle apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children);
};

//----------------------------------------------------------
class ofxNodeParentSizeLayout : public ofxNodeGuiLayout
{
public:
  ofxNodeParentSizeLayout() : sizeScalar(0,0) {}
  
  ofPoint sizeScalar;
  virtual ofRectangle apply(ofxNodeGui &parent, const std::vector<ofxNodeGui *> &children);
};

//----------------------------------------------------------
class ofxNodeGui
{
public:
  ofxNodeGui();
  ofxNodeGui(const std::string &name);
  virtual ~ofxNodeGui();

  const ofxNodeGuiStyle *getStyle() const { return this->style; }
  virtual void setStyle(ofxNodeGuiStyle *style, bool setChildren = true);

  const ofxNodeGuiLayout *getLayout() const { return this->layout; }
  void setLayout(ofxNodeGuiLayout *l) { this->layout = l; }

  const ofPoint &getPosition() const { return position; }
  void setPosition(const ofPoint &point) { position = point; }
  void setPosition(float x, float y) { position.set(x, y); }

  ofxNodeGui *getDragTarget() const { return dragTarget; }
  void setDragTarget(ofxNodeGui *point) { dragTarget = point; }

  bool getVisible() const { return visible; }
  void setVisible(bool v) { visible = v; }
  void toggleVisible() { visible = !visible;}

  bool isInteractable() const { return interactable; }
  void setInteractable(bool enabled) { interactable = enabled; }

  bool isDraggable() const { return draggable; }

  ofPoint getGlobalPosition() const
  {
    return parent ? parent->getGlobalPosition() + position : position;
  }

  const ofPoint &getSize() const { return size; }
  void setSize(const ofPoint &s) { size = s; }
  void setSize(float w, float h) { size.set(w, h); }

  const ofColor &getColor() const { return stripeColor; }
  void setColor(const ofColor &s, bool setChildren = true);

  ofxNodeGui *getParent() const { return parent; }
  void setParent(ofxNodeGui *p, bool attach = true)
  {
    parent = p;
    if (attach)
      parent->attach(this);
  }

  const std::string &getName() const { return name; }
  void setName(const std::string &n) { name = n; }

  virtual ofxNodeGui *getChildByName(const std::string &n) const;

  virtual void applyLayout();
  virtual void update();
  virtual void draw();
  virtual ofxNodeGui *hitTest(ofPoint point);
  virtual void onHover(bool hover);
  virtual void onPress(int x, int y, int button);
  virtual void onRelease(int x, int y, int button);
  virtual void onFocus(bool f) { focus = f; }
  virtual bool canFocus() const { return false; }
  virtual void setDraggable(bool enabled) { draggable = enabled; }

  virtual void attach(ofxNodeGui *child);

  virtual void keyPressed(int key);
  virtual void keyReleased(int key);

  ofEvent<void> clicked;
  
protected:
  ofxNodeGuiStyle *style;
  ofxNodeGuiLayout *layout;
  bool visible;
  bool interactable;
  bool draggable;
  bool hover;
  bool focus;

  ofxNodeGui *parent;
  ofxNodeGui *dragTarget;
  ofPoint position;
  ofPoint size;
  ofColor stripeColor;

  std::vector<ofxNodeGui *> children;
  std::string name;
};

//----------------------------------------------------------
class ofxNodeGuiCanvas : public ofxNodeGui
{
public:
  ofxNodeGuiCanvas(const std::string &name);
  virtual bool canFocus() const { return false; }

  void draw();

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseScrolled(int x, int y, float scrollX, float scrollY );

protected:
  ofxNodeGui* focusedNode;
  ofxNodeGui* lastHitNode;
  ofPoint     lastHitMousePos;
  ofPoint     scroll;
  bool lastHitDrag;
};

//----------------------------------------------------------
class ofxNodeGuiLabel : public ofxNodeGui
{
public:
  ofxNodeGuiLabel(const std::string &text);
  ofxNodeGuiLabel(const std::string &name, const std::string &text);

  virtual void applyLayout();
  virtual void update();
  virtual void draw();
  virtual ofxNodeGui *hitTest(ofPoint point);

  const std::string &getText() const { return text; }
  std::string &getText() { return text; }
  void setText(const std::string &text) { this->text = text; }

  void setSize(const ofPoint &size) { this->size = size; }
  void setSize(float x, float y) { this->size.set(x, y); }

  virtual bool canFocus() const { return false; }

protected:
  std::string text;
};

//----------------------------------------------------------
class ofxNodeGuiStripeLabel : public ofxNodeGuiLabel
{
public:
  ofxNodeGuiStripeLabel(const std::string &text);
  ofxNodeGuiStripeLabel(const std::string &name, const std::string &text);
  virtual bool canFocus() const { return false; }

  virtual void draw();
};

//----------------------------------------------------------
class ofxNodeGuiTextField : public ofxNodeGuiLabel
{
public:
  ofxNodeGuiTextField(const std::string &text);
  ofxNodeGuiTextField(const std::string &name, const std::string &text);

  virtual bool canFocus() const { return true; }

protected:
  ofAbstractParameter* parameter;
};


//----------------------------------------------------------
class ofxNodeGuiImage : public ofxNodeGui
{
public:
  ofxNodeGuiImage(const std::string &name);
  ofImage* getImage() const { return image;}
  void setImage(ofImage* i, bool useSize=false); 

  virtual void applyLayout() {}
  virtual void draw(); 
protected:
  ofImage* image;
};



//----------------------------------------------------------
class ofxNodeGuiDataVar;

//----------------------------------------------------------
class ofxTelemetry
{

public:
  class DataVar
  {
  public:
    class Sample
    {
#define Sample_offset 1e9

    public:
      float recvTime;
      float time;
      TValue_t v;

      static double makeKey(float recvtime, float sampletime)
      {
        return ((double)recvtime * Sample_offset) + sampletime;
      }

      template< typename TValueType>
      TValueType getValue(uint8_t type) const;

      template< typename TValueType>
      void setValue(uint8_t type, TValueType fv);
    };

    bool isConfig() const { return modsAllowed & Tdm_config; }
    bool canWrite() const { return modsAllowed & Tdm_write;  }

    template< typename TValueType>
    TValueType getValue() const;

    template< typename TValueType>
    void setValue(TValueType fv, bool notify=true);

    void increment(float value, bool notify=true);

    bool isFloat() const { return type == Tdt_f32;}

    bool canEdit() const { return modsAllowed & Tdm_write;}
    bool isRealtime() const { return modsAllowed & Tdm_realtime;}

    uint32_t read(ofBuffer &buf);
    uint8_t write(ofBuffer &buf) const;

    uint32_t id;
    uint8_t type;
    uint8_t modsAllowed;
    std::string shortName;
    std::string name;
    std::string desc;
    ofParameter<TValue_t> v;
    bool sendChanges;

    ofxNodeGui *guiFolder;
    ofxNodeGui *guiComp;

    std::map<double, Sample> samples;

    ofEvent<DataVar> onValueChange;
  };

  class IPayload
  {
    public:
      virtual uint32_t read(ofBuffer &buf) =0;
      virtual uint8_t write(ofBuffer &buf) const =0;
  };

  class DataVarMod : public IPayload
  {
  public:
    TDataValueMod_t mod;

    uint32_t read(ofBuffer &buf);
    uint8_t write(ofBuffer &buf) const;
  };


  ofxTelemetry();
  void update();

  uint32_t count();
  uint32_t size();

  void recv(uint8_t byte);

  void processDataMod();
  void processDataDesc();

  uint32_t writeModPacket(const TDataValueMod_t& mod, ofBuffer &buf) const;
  void onDataVarChange(DataVar& v);

  typedef enum
  {
    RECV_RESET = 0,
    RECV_MARKER,
    RECV_HEADER_TYPE,
    RECV_HEADER_SIZE,
    RECV_PAYLOAD,
    RECV_PAYLOAD_CRC
  } RecvState_t;

  RecvState_t recv_state;
  uint8_t crc;
  uint8_t offset;

  PacketHeader_t header;

  ofBuffer readBuffer;
  ofBuffer recvBuffer;
  ofBuffer sendBuffer;


  ofSerial serial;
  std::map<uint32_t, DataVar> dataVars;

  ofxNodeGui *guiLive;
  ofxNodeGui *guiConfig;
  ofxNodeGui* canvas;
};

//----------------------------------------------------------
class ofxNodeGuiDataVar : public ofxNodeGui
{
public:
  ofxNodeGuiDataVar(const std::string &name);
  
  ofxTelemetry::DataVar *getValue() const { return value; }
  virtual void setValue(ofxTelemetry::DataVar *v);

  
protected:
  ofxTelemetry::DataVar *value;
};

//----------------------------------------------------------
class ofxNodeGuiDataVarGraph: public ofxNodeGuiDataVar
{
public:
  ofxNodeGuiDataVarGraph(const std::string &name);

  virtual void applyLayout();
  virtual void update();
  virtual void draw();

  void setValue(ofxTelemetry::DataVar *v);

protected:
  virtual void onValueChange(TValue_t &v);

  ofVboMesh lineMesh;
  float lineStep;
  uint linePoints;
};

//----------------------------------------------------------
class ofxNodeGuiToggle : public ofxNodeGuiDataVar
{
public:
  ofxNodeGuiToggle(const std::string &name);
  
  virtual void applyLayout() {}
  virtual void draw(); 

  void toggleState() { value->setValue( !value->getValue<bool>()); }
protected:
};


//----------------------------------------------------------
class ofxNodeGuiNumeric : public ofxNodeGuiDataVar
{
public:
  ofxNodeGuiNumeric(const std::string &name);
  
  virtual void applyLayout() {}
  virtual void draw(); 

  void keyPressed(int key);
  void keyReleased(int key);
  void onFocus(bool f);

  bool canFocus() const { return value ? value->canEdit() : false; }


protected:
  // ofxNodeGuiLabel label;
  std::string editingText;
  int cursorIndex;
  bool editing;
};