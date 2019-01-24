#ifndef __SEMI_STRUCTURE_H__
#define __SEMI_STRUCTURE_H__

/*data structure for elements in semi-structure file*/

typedef struct
{
    char *p;
    int len;
}
xml_Text;

typedef enum {
    xml_tt_U, /* Unknow */
    xml_tt_H, /* XML Head <?xxx?>*/
    xml_tt_E, /* End Tag </xxx> */
    xml_tt_B, /* Start Tag <xxx> */
    xml_tt_BE, /* Tag <xxx/> */
    xml_tt_T, /* Content for the tag <aaa>xxx</aaa> */
    xml_tt_C, /* Comment <!--xx-->*/
    xml_tt_ATN, /* Attribute Name <xxx id="">*/
    xml_tt_ATV, /* Attribute Value <xxx id="222">*/
    xml_tt_CDATA/* <![CDATA[xxxxx]]>*/
}
xml_TokenType;

typedef struct
{
    xml_Text text;
    xml_TokenType type;
}
xml_Token;

char* convertTokenTypeToStr(xml_TokenType type); //get the type for each element
int xml_initText(xml_Text *pText, char *s); //init text
int xml_initToken(xml_Token *pToken, xml_Text *pText); //init token
char * convertTokenTypeToStr(xml_TokenType type);

#endif // !__SEMI_STRUCTURE_H__
