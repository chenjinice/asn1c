/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "MapLane"
 * 	found in "asn/MapLane.asn"
 * 	`asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -no-gen-example -D ./src`
 */

#ifndef	_ConnectingLane_H_
#define	_ConnectingLane_H_


#include <asn_application.h>

/* Including external dependencies */
#include "LaneID.h"
#include "AllowedManeuvers.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ConnectingLane */
typedef struct ConnectingLane {
	LaneID_t	 lane;
	AllowedManeuvers_t	*maneuver	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} ConnectingLane_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_ConnectingLane;
extern asn_SEQUENCE_specifics_t asn_SPC_ConnectingLane_specs_1;
extern asn_TYPE_member_t asn_MBR_ConnectingLane_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _ConnectingLane_H_ */
#include <asn_internal.h>
