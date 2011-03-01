/* dcopyright (c) 2007 magnus auvinen, see licence.txt for more info */
/* +stuff added by axel, open domain*/
/**
 * This application will extract embedded pngs from new map files.
 */
#include <stdio.h>
#include <stdlib.h>
#include <base/system.h>
#include <engine/shared/datafile.h>
#include <engine/storage.h>
#include <game/mapitems.h>

#include "./engine/external/pnglite/pnglite.h"

int main(int argc, const char **argv)
{
	if(argc != 2) {
		printf("wrong arguments, there has to be one and only one and this is the mapfile to extract the images from.\n");
		return -1;
	}
	IStorage *pStorage = CreateStorage("Teeworlds", argc, argv);
	CDataFileReader DataFile;
	
	if(!pStorage) {
		printf("Cannot get storage\n");
		return -1;
	}
	
	if(!DataFile.Open(pStorage, argv[1], IStorage::TYPE_ALL)) {
		printf("Cannot read %s\n", argv[1]);
		return -1;
	}
	
	printf("Loading %s\n", argv[1]);
	
	png_init(0, 0);
	
	// load images
	int start, num;
	DataFile.GetType(MAPITEMTYPE_IMAGE, &start, &num);
	for(int i = 0; i < num; i++)
	{
		CMapItemImage *pImg = (CMapItemImage *)DataFile.GetItem(start+i, 0, 0);
		char *pName = (char *)DataFile.GetData(pImg->m_ImageName);
		if(pImg->m_External)
		{
			printf("skipping external %s\n", pName);
		}
		else
		{
			printf("writing %s.png\n", pName);
			
			void *pData = DataFile.GetData(pImg->m_ImageData);
				
			char buf[255];
#if defined(CONF_FAMILY_WINDOWS)
			_snprintf(buf, sizeof(buf), "%s.png", pName);
#else
			snprintf(buf, sizeof(buf), "%s.png", pName);
#endif
			
			png_t png;
			png_open_file_write(&png, buf);
			png_set_data(&png, pImg->m_Width, pImg->m_Height, 8, PNG_TRUECOLOR_ALPHA, (unsigned char*) pData);
			png_close_file(&png);
			
			DataFile.UnloadData(pImg->m_ImageData);
		}
	}
	DataFile.Close();
	return 0;
}
