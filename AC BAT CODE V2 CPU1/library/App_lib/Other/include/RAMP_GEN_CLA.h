#ifndef __RAMP_GEN_CLA_H__
#define __RAMP_GEN_CLA_H__

typedef struct { float  Freq; 			// Input: Ramp frequency (pu) 	
		 	     float  StepAngleMax;	// Parameter: Maximum step angle (pu)		
	 	 	     float  Angle;			// Variable: Step angle (pu)					  
			     float  Gain;			// Input: Ramp gain (pu)
			     float  Out;  	 		// Output: Ramp signal (pu) 	
			     float  Offset;			// Input: Ramp offset (pu)
			     float  Udc;
	  	  	   } RAMP_GEN_CLA;	            

/*------------------------------------------------------------------------------
	RAMP(Sawtooh) Generator Macro Definition
------------------------------------------------------------------------------*/                                               

#define RAMP_GEN_CLA_MACRO(v)                       \
                                                    \
/* Compute the angle rate */                        \
    v.Angle += v.StepAngleMax*v.Freq;               \
                                                    \
/* Saturate the angle rate within (-1,1) */         \
    if (v.Angle>1.0f)                                \
        v.Angle -= (1.0f);                           \
    else if (v.Angle<(-1.0f))                        \
        v.Angle +=(1.0f);                            \
        v.Out=v.Angle;

#define RAMP_GEN_CLA_V2_MACRO(v)                    \
                                                    \
	/* Compute the angle rate */                    \
    v.Out = v.Out + 0.00001f;                        \
    if (v.Out > 0.1f) v.Out = 0.1f;                   \

#define RAMP_GEN_CLA_INIT(v)		\
	v.Angle=0.0f;					\
	v.Freq=0.0f;						\
	v.StepAngleMax=0.0f;				\
	v.Out=0.0f;					    \
	v.Udc = 0.0f;                    \
	
#endif // __RAMPGEN_H__
