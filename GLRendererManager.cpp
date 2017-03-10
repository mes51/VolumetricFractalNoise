#include "GLRendererManager.h"

GLRendererManager::~GLRendererManager()
{
    for (auto c : this->contexts)
    {
        c.second->SetContext();
        for (auto r : this->renderers[c.second])
        {
            r.second->DeleteBuffer();
        }
        this->shaders[c.second]->DeleteProgram();
        c.second->DeleteContext();
    }
}

std::shared_ptr<GLFilterRenderer> GLRendererManager::GetRenderer(std::thread::id index, long width, long height)
{
    std::shared_ptr<GLContext> context = GetContext(index);
    std::shared_ptr<GLFilterShaderProgram> shader = this->shaders[context];

    if (this->renderers.count(context) < 1)
    {
        this->renderers[context] = std::map<std::pair<long, long>, std::shared_ptr<GLFilterRenderer>>();
    }
    std::map<std::pair<long, long>, std::shared_ptr<GLFilterRenderer>> thisContextRenderers = this->renderers[context];

    std::pair<long, long> key = std::pair<long, long>(width, height);
    if (thisContextRenderers.count(key) < 1)
    {
        do
        {
            try
            {
                context->SetContext();
                shader->UseProgram();
                std::shared_ptr<GLFilterRenderer> renderer = std::make_shared<GLFilterRenderer>(context, shader);
                renderer->SetContext();
                renderer->ResizeBuffer(width, height);
                thisContextRenderers[key] = renderer;
                break;
            }
            catch (std::runtime_error &e)
            {
                std::pair<long, long> bk = thisContextRenderers.begin()->first;
                thisContextRenderers[bk]->DeleteBuffer();
                thisContextRenderers.erase(bk);
            }
        } while (thisContextRenderers.size() > 0);
        this->renderers[context] = thisContextRenderers;
    }
    if (thisContextRenderers.size() > 0)
    {
        return thisContextRenderers[key];
    }
    else
    {
        throw std::runtime_error("cannot create renderer");
    }
}

std::shared_ptr<GLContext> GLRendererManager::GetContext(std::thread::id index)
{
    if (this->contexts.count(index) < 1)
    {
        std::shared_ptr<GLContext> context = std::make_shared<GLContext>(this->rootContext);
        context->SetContext();
        std::shared_ptr<GLFilterShaderProgram> shader = std::make_shared<GLFilterShaderProgram>(this->fragmentShaderCode);
        this->contexts[index] = context;
        this->shaders[context] = shader;
    }
    return this->contexts[index];
}
