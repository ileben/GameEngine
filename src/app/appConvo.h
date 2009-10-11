#ifndef __APPCONVO_H
#define __APPCONVO_H

#include <util/geUtil.h>
#include <engine/geEngine.h>
using namespace GE;

class Convo;
class ConvoNode;
class ConvoSpeach;
class ConvoBranch;
class ConvoOpt;
class ConvoSpeaker;
class ConvoOptLabel;
class ConvoSpeachLabel;

typedef void (*ConvoFunc) (Convo *convo, ConvoNode *convoNode);

class ConvoNode : public Object
{
  DECLARE_SUBCLASS( ConvoNode, Object );
  DECLARE_END;

public:
  Float duration;
  ConvoNode *next;
  ConvoFunc startFunc;
  ConvoFunc endFunc;

  ConvoNode() : next(NULL), duration(0.0), startFunc(NULL), endFunc(NULL) {}

  ConvoSpeach* addSpeach (ConvoSpeaker *speaker, Float dur, const CharString &text,
    ConvoFunc startFunc = NULL, ConvoFunc endFunc = NULL);

  ConvoBranch* addBranch (Float dur);
};

class ConvoSpeach : public ConvoNode
{
  DECLARE_SUBCLASS( ConvoSpeach, ConvoNode );
  DECLARE_END;

public:
  ConvoSpeaker *speaker;
  CharString text;

  ConvoSpeach () : speaker(NULL) {}

  ConvoSpeach (ConvoSpeaker *speaker, Float dur, const CharString &text,
    ConvoFunc startFunc = NULL, ConvoFunc endFunc = NULL);
};

class ConvoBranch : public ConvoNode
{
  DECLARE_SUBCLASS( ConvoBranch, ConvoNode );
  DECLARE_END;

public:
  ArrayList <ConvoOpt*> options;

  ConvoOpt* addOption (const CharString &title);
};

class ConvoOpt : public ConvoNode
{
public:
  CharString text;
};

class ConvoSpeaker
{
public:
  Vector2 loc;
  Vector3 color;
};

class Convo : public ConvoNode
{
  friend class ConvoOptLabel;

  Float time;
  ConvoNode *node;
  Actor *actor;
  Scene *scene;
  void show();
  void hide();
  void advance (ConvoNode *to);

public:
  void bindScene (Scene *scene);
  void begin();
  void skip();
  void tick();
  void end();
  bool done();

  Convo() : node(NULL), actor(NULL), scene(NULL) {}
};


class ConvoOptLabel : public Label
{
public:
  Vector3 outColor;
  Vector3 inColor;
  Convo *convo;
  ConvoOpt *opt;

  virtual void onMouseClick (MouseClickEvent *e);
  virtual void onMouseEnter (MouseEnterEvent *e);
  virtual void onMouseLeave (MouseLeaveEvent *e);
};

class ConvoSpeachLabel : public Label
{
public:
  virtual void draw();
};

#endif//__APPCONVO_H
