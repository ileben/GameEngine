#include <app/appConvo.h>
#include <engine/geGLHeaders.h>

DEFINE_CLASS( ConvoNode );
DEFINE_CLASS( ConvoSpeach );
DEFINE_CLASS( ConvoBranch );

void Convo::bindScene (Scene *s)
{
  scene = s;
}

void Convo::show()
{
  if (node == NULL) return;

  //Invoke start callback
  if (node->startFunc != NULL)
    node->startFunc( this, node );

  //Create actor
  if (ClassOf( node ) == Class( ConvoSpeach ))
  {
    ConvoSpeach* speach = (ConvoSpeach*) node;
    Vector2 winSize = Kernel::GetInstance()->getRenderer()->getWindowSize();

    ConvoSpeachLabel *lbl = new ConvoSpeachLabel;
    lbl->setText( speach->text );
    lbl->setLoc( 10, winSize.y - 50 );
    lbl->setColor( speach->speaker->color );
    lbl->setParent( scene->getRoot() );

    actor = lbl;
  }
  else if (ClassOf( node ) == Class( ConvoBranch ))
  {
    ConvoBranch* branch = (ConvoBranch*) node;

    Actor *grp = new Actor;
    grp->setParent( scene->getRoot() );

    for (UintSize o=0; o<branch->options.size(); ++o)
    {
      ConvoOpt* opt = branch->options[o];

      ConvoOptLabel *lbl = new ConvoOptLabel;
      lbl->opt = opt;
      lbl->convo = this;
      lbl->outColor = Vector3(1,1,1);
      lbl->inColor = Vector3(1,1,0);
      lbl->setText( opt->text );
      lbl->setLoc( 10, 20 + o * 20 );
      lbl->setColor( lbl->outColor );
      lbl->setParent( grp );
    }

    actor = grp;
  }
}

void Convo::hide()
{
  //Destroy actor
  if (actor != NULL)
    actor->destroy();

  //Invoke end callback
  if (node != NULL)
    if (node->endFunc != NULL)
      node->endFunc( this, node );
}

void Convo::advance (ConvoNode *to)
{
  hide();
  node = to;
  show();

  //Mark the starting time of this node
  time = Kernel::GetInstance()->getTime();
}

void Convo::begin()
{
  advance( next );
}

void Convo::end()
{
  node = NULL;
  hide();
}

bool Convo::done()
{
  return (node == NULL);
}

void Convo::skip ()
{
  if (node == NULL) return;

  //Branches cannot be skipped
  if (ClassOf( node ) != Class( ConvoBranch ))
    advance( node->next );
}

void Convo::tick()
{
  if (node == NULL) return;

  //Check if the node ended
  Float newTime = Kernel::GetInstance()->getTime();
  if (newTime - time > node->duration)
    advance( node->next );
}


/*
----------------------------------
Construction helpers
----------------------------------*/

ConvoSpeach* ConvoNode::addSpeach (ConvoSpeaker *speaker, Float dur,
  const CharString &text, ConvoFunc startFunc, ConvoFunc endFunc)
{
  ConvoSpeach *speach = new ConvoSpeach;
  speach->speaker = speaker;
  speach->duration = dur;
  speach->text = text;
  speach->startFunc = startFunc;
  speach->endFunc = endFunc;

  next = speach;
  return speach;
}

ConvoBranch* ConvoNode::addBranch (Float dur)
{
  ConvoBranch *branch = new ConvoBranch;
  branch->duration = dur;

  next = branch;
  return branch;
}

ConvoOpt* ConvoBranch::addOption (const GE::CharString &title)
{
  ConvoOpt *opt = new ConvoOpt;
  opt->text = title;
  options.pushBack( opt );
  return opt;
}

/*
----------------------------------------
Conversation widgets
----------------------------------------*/

void ConvoOptLabel::onMouseClick (MouseClickEvent *e)
{
  //Advance to node belonging to this option label
  convo->advance( opt->next );
}

void ConvoOptLabel::onMouseEnter (MouseEnterEvent *e)
{
  setColor( inColor);
}

void ConvoOptLabel::onMouseLeave (MouseLeaveEvent *e)
{
  setColor( outColor );
}

void ConvoSpeachLabel::draw()
{
  Vector2 winSize = Kernel::GetInstance()->getRenderer()->getWindowSize();

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glColor4f( 0,0,0, 0.8f );

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glTranslatef( 0, winSize.y - 60, 0 );
  glScalef( winSize.x, 60, 1 );

  glBegin( GL_QUADS );
  glVertex2f(0,0);
  glVertex2f(1,0);
  glVertex2f(1,1);
  glVertex2f(0,1);
  glEnd();

  glPopMatrix();

  glDisable( GL_BLEND );

  Label::draw();
}
