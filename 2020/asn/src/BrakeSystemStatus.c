/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "VehBrake"
 * 	found in "asn/VehBrake.asn"
 * 	`asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -no-gen-example -D ./src`
 */

#include "BrakeSystemStatus.h"

asn_TYPE_member_t asn_MBR_BrakeSystemStatus_1[] = {
	{ ATF_POINTER, 7, offsetof(struct BrakeSystemStatus, brakePadel),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BrakePedalStatus,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"brakePadel"
		},
	{ ATF_POINTER, 6, offsetof(struct BrakeSystemStatus, wheelBrakes),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BrakeAppliedStatus,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"wheelBrakes"
		},
	{ ATF_POINTER, 5, offsetof(struct BrakeSystemStatus, traction),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TractionControlStatus,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"traction"
		},
	{ ATF_POINTER, 4, offsetof(struct BrakeSystemStatus, abs),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AntiLockBrakeStatus,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"abs"
		},
	{ ATF_POINTER, 3, offsetof(struct BrakeSystemStatus, scs),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_StabilityControlStatus,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"scs"
		},
	{ ATF_POINTER, 2, offsetof(struct BrakeSystemStatus, brakeBoost),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BrakeBoostApplied,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"brakeBoost"
		},
	{ ATF_POINTER, 1, offsetof(struct BrakeSystemStatus, auxBrakes),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_AuxiliaryBrakeStatus,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"auxBrakes"
		},
};
static const int asn_MAP_BrakeSystemStatus_oms_1[] = { 0, 1, 2, 3, 4, 5, 6 };
static const ber_tlv_tag_t asn_DEF_BrakeSystemStatus_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_BrakeSystemStatus_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* brakePadel */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* wheelBrakes */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* traction */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* abs */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* scs */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* brakeBoost */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 } /* auxBrakes */
};
asn_SEQUENCE_specifics_t asn_SPC_BrakeSystemStatus_specs_1 = {
	sizeof(struct BrakeSystemStatus),
	offsetof(struct BrakeSystemStatus, _asn_ctx),
	asn_MAP_BrakeSystemStatus_tag2el_1,
	7,	/* Count of tags in the map */
	asn_MAP_BrakeSystemStatus_oms_1,	/* Optional members */
	7, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_BrakeSystemStatus = {
	"BrakeSystemStatus",
	"BrakeSystemStatus",
	&asn_OP_SEQUENCE,
	asn_DEF_BrakeSystemStatus_tags_1,
	sizeof(asn_DEF_BrakeSystemStatus_tags_1)
		/sizeof(asn_DEF_BrakeSystemStatus_tags_1[0]), /* 1 */
	asn_DEF_BrakeSystemStatus_tags_1,	/* Same as above */
	sizeof(asn_DEF_BrakeSystemStatus_tags_1)
		/sizeof(asn_DEF_BrakeSystemStatus_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_BrakeSystemStatus_1,
	7,	/* Elements count */
	&asn_SPC_BrakeSystemStatus_specs_1	/* Additional specs */
};

