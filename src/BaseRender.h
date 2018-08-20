#ifndef BASE_RENDER_H
#define BASE_RENDER_H
class BaseRender 
{
public:
    BaseRender() = default;
    ~BaseRender() = default;
    virtual void Render() = 0;
};
#endif // BASE_RENDER_H