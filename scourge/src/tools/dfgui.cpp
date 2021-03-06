#include "dfgui.h"
#include "../common/constants.h"

/*DFGui::DFGui()
{
	//ctor
}

DFGui::~DFGui()
{
	//dtor
}*/

bool DFGui::LoadSingle(std::ifstream *fin, Theme *theme)
{
	// read name
	char buffer[256];
	fin->getline(buffer, 256, '\n');
	theme->name = &(buffer[2]);		// Skip the "T:"

	ParseElement(fin,	theme->elements["windowBack"]);
	ParseElement(fin,	theme->elements["windowTop"]);
	ParseElement(fin,	theme->elements["windowBorder"]);

	ParseColor(fin,	theme->colors["windowTitleText"]);
	ParseColor(fin,	theme->colors["windowText"]);

	ParseElement(fin,	theme->elements["buttonBackground"]);
	ParseElement(fin,	theme->elements["buttonSelectionBackground"]);
	ParseElement(fin,	theme->elements["buttonHighlight"]);
	ParseElement(fin,	theme->elements["buttonBorder"]);

	ParseColor(fin,	theme->colors["buttonText"]);
	ParseColor(fin,	theme->colors["buttonSelectionText"]);

	ParseElement(fin,	theme->elements["listBackground"]);
	ParseElement(fin,	theme->elements["inputBackground"]);

	ParseColor(fin,	theme->colors["inputText"]);

	ParseElement(fin,	theme->elements["selectionBackground"]);

	ParseColor(fin,	theme->colors["selectionText"]);

	ParseElement(fin,	theme->elements["selectedBorder"]);
	ParseElement(fin,	theme->elements["selectedCharacterBorder"]);

	ParseTexturedBorder(fin, theme->texturedBorder);

	return true;
}

bool DFGui::ParseElement(std::ifstream *fin, Element *element)
{
	char buffer[256]; char *p = buffer+2;	// skip the "T:"
	fin->getline(buffer, 256, '\n');

	std::cerr << "\nELEMENT";
	std::cerr << "\nbuffer = " << buffer;

	if ( strtok(p, ",") == 0 ) return true;		// Blank row

	Color *c = &element->color;

	element->texture = p;
	c->r = atof( strtok(0, ",") );
	c->g = atof( strtok(0, ",") );
	c->b = atof( strtok(0, ",") );
	c->a = atof( strtok(0, ",") );
	element->lineWidth = atoi( strtok(0, ",") );

	if ( (p = strtok(0, ",")) ){
		element->north = p;std::cerr<<"\nnorth: p="<<p<<'\n';}
	if ( (p = strtok(0, ",")) ){
		element->south = p;std::cerr<<"\nsouth: p="<<p<<'\n';}
	if ( (p = strtok(0, ",")) ){
		element->east = p;std::cerr<<"\neast: p="<<p<<'\n';}
	if ( (p = strtok(0, ", #\t")) ){
		element->west = p;std::cerr<<"\nwest: p="<<p<<'\n';}

	std::cerr << "\ntexture = " << element->texture;
	std::cerr << "\nred = " << c->r;
	std::cerr << "\ngreen = " << c->g;
	std::cerr << "\nblue = " << c->b;
	std::cerr << "\nalpha = " << c->a;
	std::cerr << "\nwidth = " << element->lineWidth;
	std::cerr << "\nnorth = " << element->north;
	std::cerr << "\nsouth = " << element->south;
	std::cerr << "\neast = " << element->east;
	std::cerr << "\nwest = " << element->west << "\n\n";

	return true;
}

bool DFGui::ParseColor(std::ifstream *fin, Color *color)
{
	char buffer[256]; char *p = buffer+2;
	fin->getline(buffer, 256, '\n');

	std::cerr << "\nCOLOR";
	std::cerr << "\nbuffer = " << buffer;

	if ( strtok(p, ",") == 0 ) return true;

	color->r = atof( p );
	color->g = atof( strtok(0, ",") );
	color->b = atof( strtok(0, ",") );
	color->a = atof( strtok(0, ", #") );

	std::cerr << "\nred = " << color->r;
	std::cerr << "\ngreen = " << color->g;
	std::cerr << "\nblue = " << color->b;
	std::cerr << "\nalpha = " << color->a;

	return true;
}

bool DFGui::ParseTexturedBorder(std::ifstream *fin, TexturedBorder *texturedBorder)
{
	char buffer[256]; char *p = buffer+2;	// skip the "T:"
	fin->getline(buffer, 256, '\n');

	std::cerr << "\nELEMENT";
	std::cerr << "\nbuffer = " << buffer;

	if ( strtok(p, ",") == 0 ) return true;		// Blank row

	Color *c = &texturedBorder->color;

	texturedBorder->texture = p;
	c->r = atof( strtok(0, ",") );
	c->g = atof( strtok(0, ",") );
	c->b = atof( strtok(0, ",") );
	c->a = atof( strtok(0, ",") );
	texturedBorder->lineWidth = atoi( strtok(0, ",") );

	if ( (p = strtok(0, ",")) ){
		texturedBorder->north = p;std::cerr<<"\nnorth: p="<<p;}
	if ( (p = strtok(0, ",")) ){
		texturedBorder->south = p;std::cerr<<"\nsouth: p="<<p;}
	if ( (p = strtok(0, ",")) ){
		texturedBorder->east = p;std::cerr<<"\neast: p="<<p;}
	if ( (p = strtok(0, ", #\t")) ){
		texturedBorder->west = p;std::cerr<<"\nwest: p="<<p;}
	if ( (p = strtok(0, ",")) ){
		texturedBorder->nw = p;std::cerr<<"\nnorth west: p="<<p;}
	if ( (p = strtok(0, ",")) ){
		texturedBorder->ne = p;std::cerr<<"\nnorth east: p="<<p;}
	if ( (p = strtok(0, ", #\t")) ){
		texturedBorder->sw = p;std::cerr<<"\nsouth west: p="<<p;}
	if ( (p = strtok(0, ", #\t")) ){
		texturedBorder->se = p;std::cerr<<"\nsouth east: p="<<p;}

	std::cerr << "\ntexture = " << texturedBorder->texture;
	std::cerr << "\nred = " << c->r;
	std::cerr << "\ngreen = " << c->g;
	std::cerr << "\nblue = " << c->b;
	std::cerr << "\nalpha = " << c->a;
	std::cerr << "\nwidth = " << texturedBorder->lineWidth;
	std::cerr << "\nnorth = " << texturedBorder->north;
	std::cerr << "\nsouth = " << texturedBorder->south;
	std::cerr << "\neast = " << texturedBorder->east;
	std::cerr << "\nwest = " << texturedBorder->west << "\n\n";

	return true;
}

void DFGui::Save()
{
	std::ofstream fout( GetDataPath("%s/world/guiTEST"), std::ios::binary);

	fout << "# gui themes key:\n# (non-existing textures will appear as pure color)"
		 << "\n#\n# T:theme name\n# each row is either an \"element\" (like window background) or a \"color\" like a text color."
		 << "\n# to skip a row, enter something w/o commas like \"skip\".\n# use the default theme as an example."
		 << "\n#\n# Elements have: texture,r,g,b,a,line_width,north_texture,south,east,west"
		 << "\n#\n# Colors have: r,g,b,a\n# darkuitop\n\n";

	std::vector <Theme*>::iterator itr;
	for ( itr = data.begin(); itr != data.end(); itr++ )
	{
		Theme *theme = *itr;

		fout << "T:" << theme->name;

		SaveElement(fout, theme, "windowBack");
		SaveElement(fout, theme, "windowTop");
		SaveElement(fout, theme, "windowBorder");
			SaveColor(fout, theme, "windowTitleText");
			SaveColor(fout, theme, "windowText");
		SaveElement(fout, theme, "buttonBackground");
		SaveElement(fout, theme, "buttonSelectionBackground");
		SaveElement(fout, theme, "buttonHighlight");
		SaveElement(fout, theme, "buttonBorder");
			SaveColor(fout, theme, "buttonText");
			SaveColor(fout, theme, "buttonSelectionText");
		SaveElement(fout, theme, "listBackground");
		SaveElement(fout, theme, "inputBackground");
			SaveColor(fout, theme, "inputText");
		SaveElement(fout, theme, "selectionBackground");
			SaveColor(fout, theme, "selectionText");
		SaveElement(fout, theme, "selectedBorder");
		SaveElement(fout, theme, "selectedCharacterBorder");

		SaveTexturedBorder(fout ,theme->texturedBorder);

		fout << "\n\n";
	}

	fout.close();
}

void DFGui::SaveElement(std::ofstream &fout, Theme *theme, char *elementName)
{
	char buffer[256];
	Element *e = theme->elements[elementName];	Color *c = &e->color;

	if ( e->texture == "" )		e->texture = "none";
	if ( e->north == "" )		e->north = "none";
	if ( e->south == "" )		e->south = "none";
	if ( e->east == "" )		e->east = "none";
	if ( e->west == "" )		e->west = "none";

	sprintf(buffer, "T:%s,%.3f,%.3f,%.3f,%.3f,%i,%s,%s,%s,%s\t\t# %s", e->texture.c_str(), c->r,c->g,c->b,c->a,e->lineWidth,
			e->north.c_str(),e->south.c_str(),e->east.c_str(),e->west.c_str(), elementName);
	fout << "\n" << buffer;
std::cerr << "\n" << buffer << "\n";
}
void DFGui::SaveColor(std::ofstream &fout, Theme *theme, char *colorName)
{
	char buffer[256];
	Color *c = theme->colors[colorName];
		sprintf(buffer, "T:,%.3f,%.3f,%.3f,%.3f", c->r,c->g,c->b,c->a);
		fout << "\n" << buffer;
std::cerr << "\n" << buffer << "\n";
}
void DFGui::SaveTexturedBorder(std::ofstream &fout ,TexturedBorder *texturedBorder)
{
	char buffer[256];
	TexturedBorder *t = texturedBorder;		Color *c = &t->color;

	if ( t->texture == "" )		t->texture = "none";
	if ( t->north == "" )		t->north = "none";
	if ( t->south == "" )		t->south = "none";
	if ( t->east == "" )		t->east = "none";
	if ( t->west == "" )		t->west = "none";
	if ( t->nw == "" )		t->nw = "none";
	if ( t->ne == "" )		t->ne = "none";
	if ( t->sw == "" )		t->sw = "none";
	if ( t->se == "" )		t->se = "none";

	sprintf(buffer, "T:%s,%.3f,%.3f,%.3f,%.3f,%i,%s,%s,%s,%s,%s,%s,%s,%s\t\t# textured border", t->texture.c_str(), c->r,c->g,c->b,c->a,t->lineWidth,
			t->north.c_str(),t->south.c_str(),t->east.c_str(),t->west.c_str(), t->nw.c_str(),t->ne.c_str(),t->sw.c_str(),t->se.c_str());
	fout << "\n" << buffer;
std::cerr << "\n" << buffer << "\n";
}
