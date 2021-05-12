/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "LTEV"
 * 	found in "LTEV.asn"
 * 	`asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -D ./src`
 */

#ifndef	_BrakeSystemStatusltev_H_
#define	_BrakeSystemStatusltev_H_


#include <asn_application.h>

/* Including external dependencies */
#include "BrakePedalStatus.h"
#include "BrakeAppliedStatusltev.h"
#include "TractionControlStatus.h"
#include "AntiLockBrakeStatusltev.h"
#include "StabilityControlStatusltev.h"
#include "BrakeBoostAppliedltev.h"
#include "AuxiliaryBrakeStatusltev.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BrakeSystemStatusltev */
typedef struct BrakeSystemStatusltev {
	BrakePedalStatus_t	*brakePadel	/* OPTIONAL */;
	BrakeAppliedStatusltev_t	*wheelBrakes	/* OPTIONAL */;
	TractionControlStatus_t	*traction	/* OPTIONAL */;
	AntiLockBrakeStatusltev_t	*abs	/* OPTIONAL */;
	StabilityControlStatusltev_t	*scs	/* OPTIONAL */;
	BrakeBoostAppliedltev_t	*brakeBoost	/* OPTIONAL */;
	AuxiliaryBrakeStatusltev_t	*auxBrakes	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BrakeSystemStatusltev_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BrakeSystemStatusltev;
extern asn_SEQUENCE_specifics_t asn_SPC_BrakeSystemStatusltev_specs_1;
extern asn_TYPE_member_t asn_MBR_BrakeSystemStatusltev_1[7];

#ifdef __cplusplus
}
#endif

#endif	/* _BrakeSystemStatusltev_H_ */
#include <asn_internal.h>