/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SPATIntersectionState"
 * 	found in "asn/SPATIntersectionState.asn"
 * 	`asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -no-gen-example -D ./src`
 */

#include "UTCTiming.h"

asn_TYPE_member_t asn_MBR_UTCTiming_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct UTCTiming, startUTCTime),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeMark,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"startUTCTime"
		},
	{ ATF_POINTER, 2, offsetof(struct UTCTiming, minEndUTCTime),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeMark,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"minEndUTCTime"
		},
	{ ATF_POINTER, 1, offsetof(struct UTCTiming, maxEndUTCTime),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeMark,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"maxEndUTCTime"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct UTCTiming, likelyEndUTCTime),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeMark,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"likelyEndUTCTime"
		},
	{ ATF_POINTER, 3, offsetof(struct UTCTiming, timeConfidence),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Confidence,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"timeConfidence"
		},
	{ ATF_POINTER, 2, offsetof(struct UTCTiming, nextStartUTCTime),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeMark,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"nextStartUTCTime"
		},
	{ ATF_POINTER, 1, offsetof(struct UTCTiming, nextEndUTCTime),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeMark,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"nextEndUTCTime"
		},
};
static const int asn_MAP_UTCTiming_oms_1[] = { 1, 2, 4, 5, 6 };
static const ber_tlv_tag_t asn_DEF_UTCTiming_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_UTCTiming_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* startUTCTime */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* minEndUTCTime */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* maxEndUTCTime */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* likelyEndUTCTime */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* timeConfidence */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* nextStartUTCTime */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 } /* nextEndUTCTime */
};
asn_SEQUENCE_specifics_t asn_SPC_UTCTiming_specs_1 = {
	sizeof(struct UTCTiming),
	offsetof(struct UTCTiming, _asn_ctx),
	asn_MAP_UTCTiming_tag2el_1,
	7,	/* Count of tags in the map */
	asn_MAP_UTCTiming_oms_1,	/* Optional members */
	5, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_UTCTiming = {
	"UTCTiming",
	"UTCTiming",
	&asn_OP_SEQUENCE,
	asn_DEF_UTCTiming_tags_1,
	sizeof(asn_DEF_UTCTiming_tags_1)
		/sizeof(asn_DEF_UTCTiming_tags_1[0]), /* 1 */
	asn_DEF_UTCTiming_tags_1,	/* Same as above */
	sizeof(asn_DEF_UTCTiming_tags_1)
		/sizeof(asn_DEF_UTCTiming_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_UTCTiming_1,
	7,	/* Elements count */
	&asn_SPC_UTCTiming_specs_1	/* Additional specs */
};

