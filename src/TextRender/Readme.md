TextRender
===
TextRender is a class for rendering text. It refers to the Klay Engine.

It uses the SDF(Signed Distance Field) to render the font. The class needs a prepared file which is produced by Kfont.exe to initialize. Kfont.exe is a tool of KlayGE and could convert a ttf file to a kfont file using freetype-gl. 

TODO:

+ Read the KlayGE code about generating kfont and build my own code
+ generate the kfont file in real-time