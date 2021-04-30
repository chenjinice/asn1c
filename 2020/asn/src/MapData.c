/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Map"
 * 	found in "asn/Map.asn"
 * 	`asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -no-gen-example -D ./src`
 */

#include "MapData.h"

asn_TYPE_member_t asn_MBR_MapData_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct MapData, msgCnt),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MsgCount,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"msgCnt"
		},
	{ ATF_POINTER, 1, offsetof(struct MapData, timeStamp),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_MinuteOfTheYear,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"timeStamp"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct MapData, nodes),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NodeList,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"nodes"
		},
};
static const int asn_MAP_MapData_oms_1[] = { 1 };
static const ber_tlv_tag_t asn_DEF_MapData_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_MapData_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* msgCnt */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* timeStamp */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* nodes */
};
asn_SEQUENCE_specifics_t asn_SPC_MapData_specs_1 = {
	sizeof(struct MapData),
	offsetof(struct MapData, _asn_ctx),
	asn_MAP_MapData_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_MapData_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	3,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_MapData = {
	"MapData",
	"MapData",
	&asn_OP_SEQUENCE,
	asn_DEF_MapData_tags_1,
	sizeof(asn_DEF_MapData_tags_1)
		/sizeof(asn_DEF_MapData_tags_1[0]), /* 1 */
	asn_DEF_MapData_tags_1,	/* Same as above */
	sizeof(asn_DEF_MapData_tags_1)
		/sizeof(asn_DEF_MapData_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_MapData_1,
	3,	/* Elements count */
	&asn_SPC_MapData_specs_1	/* Additional specs */
};

