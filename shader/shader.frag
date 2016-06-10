#version 430
layout (early_fragment_tests) in;

#define MAX_FRAGS 75

#define INSERTION
//#define BUBBLE
//#define MERGE
//#define SHELL
//#define SELECT

//#define USE_TEXT

in vec3 outPosition;
in vec3 outNormal;
in vec2 outUV;

uniform sampler2D texImage;
uniform vec3 bgColor;

vec4 LightPosition = vec4(vec3(0.0), 1.0);
vec3 LightIntensity = vec3(1.0);

uniform vec4 Kd;

struct LLNode {
	vec4 mColor;
	float mDepth;
	uint next;
};

layout( binding = 0, r32ui) uniform uimage2D headPointers;
layout( binding = 0, offset = 0) uniform atomic_uint nextNodeCounter;
layout( binding = 0, std430 ) buffer lilit {
	LLNode nodes[];
};

uniform uint maxNodes;

layout( location = 0 ) out vec4 outColor;

subroutine void RPType();
subroutine uniform RPType RenderPass;

vec3 iluminati( ) {
	vec3 s = normalize( LightPosition.xyz - outPosition );
	vec3 n = normalize(outNormal);
	/*if(!gl_FrontFacing) {
		n = -n;
	}*/
	#ifdef USE_TEXT
		vec3 color = texture(texImage, outUV).rgb;
	#else
		vec3 color = Kd.rgb;
	#endif
	
	float levels = 5.0;
	//return LightIntensity * color * floor( max( dot(s, n), 0.0 ) * levels) * (1.0 / levels);
	return LightIntensity * color * max( dot(s, n), 0.0 );
}

//#define INFINITY 1.0/0.0

/*#ifdef RADIX
LLNode findLargestDepth(LLNode frags[MAX_FRAGS], int fragCount) {
	int i;
	LLNode largest = frags[0];
	//float largestDepth = -INFINITY;
	for( i = 1; < i < fragCount; i++ ) {
		if(frags[i].mDepth > largest.mDepth) {
			largest = frags[i];
		}
	}
	return largest;
}
#endif*/

subroutine(RPType)
void pass1() {
	uint nodeIdx = atomicCounterIncrement(nextNodeCounter);

	if( nodeIdx < maxNodes ) {
		uint prevHead = imageAtomicExchange(headPointers, ivec2(gl_FragCoord.xy), nodeIdx);

		nodes[nodeIdx].mColor = vec4(iluminati(), Kd.a);
		nodes[nodeIdx].mDepth = gl_FragCoord.z;
		nodes[nodeIdx].next = prevHead;
	}
}

subroutine(RPType)
void pass2() {
	LLNode frags[MAX_FRAGS];
	ivec2 pos = ivec2(gl_FragCoord.xy);
	int fragCount = 0;

	uint hIdx = imageLoad(headPointers, pos).x;

	while( hIdx != 0xffffffff && fragCount < MAX_FRAGS) {
		frags[fragCount] = nodes[hIdx];
		hIdx = frags[fragCount].next;
		fragCount++;
	}

	#ifdef RADIX
	/*// Get maximum value of mDepth
	float mx = frags[0].mDepth;
	for(int i = 1; i < fragCount; i++) {
		if(frags[i].mDepth > mx) {
			mx = frags[i].mDepth;
		}
	}
	int exp = 1;
	while(mx/exp > 0) {
		LLNode output[fragCount];
		int i, count[10];
		for(i = 0; i < fragCount; i++) {
			count[
		}

		exp *= 10;
	}*/
	#endif

	// https://github.com/OpenGLInsights/OpenGLInsightsCode/blob/master/Chapter%2020%20Efficient%20Layered%20Fragment%20Buffer%20Techniques/sorting.glsl
	#ifdef SHELL
		int inc = fragCount / 2;
		int i, j;
		LLNode tmp;
		while (inc > 0) {
			for (i = inc; i < fragCount; i++) {
				tmp = frags[i];
				j = i;
				while (j >= inc && frags[j - inc].mDepth < tmp.mDepth) {
					frags[j] = frags[j - inc];
					j -= inc;
				}
				frags[j] = tmp;
			}
			inc = int(inc / 2.2 + 0.5);
		}
	#endif

	#ifdef SELECT
		LLNode aux;
		int i, j, swap;
		for (j = 0; j < fragCount-1; j++) {
			swap = j;
			for (i = j+1; i < fragCount; i++) {
				if (frags[swap].mDepth < frags[i].mDepth) {
					swap = i;
				}
			}
			aux = frags[swap];
			frags[swap] = frags[j];
			frags[j] = aux;
		}
	#endif

	#ifdef INSERTION
		LLNode aux;
 		for(uint i = 1; i < fragCount; i++) {
			aux = frags[i];
			uint j = i;
			while(j > 0 && aux.mDepth > frags[j-1].mDepth) {
				frags[j] = frags[j-1];
				j--;
			}
			frags[j] = aux;
		}
	#endif
	
	#ifdef BUBBLE
		int j, i;
		LLNode tempNode;
		for(i = 0; i < fragCount - 1; i++) {
			for(j = 0; j < fragCount - i - 1; j++) {
				if(frags[j].mDepth < frags[j+1].mDepth) {
					tempNode = frags[j];
					frags[j] = frags[j+1];
					frags[j+1] = tempNode;
				}
			}
		}
	#endif

	#ifdef MERGE
		int i, j1, j, i_merge, k;
		int a, b, c;
		int _step_ = 1;
		LLNode left_array[MAX_FRAGS/2];
    
		while (_step_ <= fragCount) {
			i = 0;
			while (i < fragCount - _step_) {
				a = i;
				b = i + _step_;
				c = min(i + _step_ + _step_, fragCount);

				for (i_merge = 0; i_merge < _step_; i_merge++)
					left_array[i_merge] = frags[a + i_merge];
            
				i_merge = 0;
				j = 0;
				for (int k = a; k < c; k++) {
					if (b + i_merge >= c || (j < _step_ && left_array[j].mDepth > frags[b + i_merge].mDepth)) {
						frags[k] = left_array[j++];
					} else {
						frags[k] = frags[b + i_merge++];
					}
				}
				i += 2 * _step_;
			}
			_step_ *= 2;
		} 
	#endif
	/*#ifdef RADIX
		int i;
		int semiSorted[MAX_FRAGS];
		float significantDigit = 1.0;
		LLNode largestDepth = findLargestDepth(frags, fragCount);

		while(largestDepth.mDepth / significantDigit > 0.0) {
			int bucket[10];
			for(i = 0; i < fragCount; i++) {
				bucket[(frags[i] / significantDigit) % 10]++;
			}
		}
	#endif*/

	vec4 color = vec4(bgColor, 1.0);
	for(uint i = 0; i < fragCount; i++) {
		color = mix(color, frags[i].mColor, frags[i].mColor.a);
	}
	outColor = color;
}

void main() {
	RenderPass();
}