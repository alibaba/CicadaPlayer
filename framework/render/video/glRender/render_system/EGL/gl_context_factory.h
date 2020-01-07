#ifndef GL_CONTEXT_FACTORY_H
#define GL_CONTEXT_FACTORY_H
#include "gl_context.h"
namespace Cicada
{

class GLContextFactory
{
public:
    static GLContext* NewInstance();
};

} // namespace cicada
#endif // GL_CONTEXT_FACTORY_H
