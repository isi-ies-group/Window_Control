typedef struct {
	int tilt_correction;
	double azimuth;
	double elevation;
	double pan;
	double tilt;
}AOIInputs;


void aoicalcTask(void *pvParameters);