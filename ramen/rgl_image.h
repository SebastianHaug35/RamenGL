#ifndef RGL_IMAGE_H
#define RGL_IMAGE_H

class Image
{
  public:
    Image();
    bool           Load(const char* file);
    int            GetWidth();
    int            GetHeight();
    unsigned char* Data();

  private:
    int            m_Width;
    int            m_Height;
    unsigned char* m_Data;
};

#endif
