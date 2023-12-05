---
title: 'Generating Gravitational-Wave Signals with LALSimulation and gwsignal'
tags:
  - Python
  - astronomy
  - gravitational waves
authors:
  - name: John Doe
    orcid: 
    affiliation: "1, 2" # (Multiple affiliations must be quoted)
affiliations:
 - name: Institution Name, Country
   index: 1
 - name: Independent Researcher, Country
   index: 1
date: 22 November 2023
bibliography: paper.bib

# Optional fields if submitting to a AAS journal too, see this blog post:
# https://blog.joss.theoj.org/2018/12/a-new-collaboration-with-aas-publishing
aas-doi: 10.3847/xxxxx <- update this with the DOI from AAS once you know it.
aas-journal: Astrophysical Journal <- The name of the AAS journal.
---

# Summary

Summary comes here.

# C interface
The C interface is written in the same language as the traditional LALSimulation, the C99 language. It aims to enhance the waveform generation functionalities by introducing several new features. This interface allows to generate the gravitational wave polarizations and individual modes of all the models implemented in LALSimulation, but also allows the evaluation of external waveform models implemented both in C or in the Python. Furthermore, the interface introduces the flexibility to specify different combinations of input arguments through a dictionary-like object instead of providing a fixed list of parameters. 

In order to leverage the full set of new functionalities, the user will need to switch to a new set of standard functions that superseed the waveform generation functions used so far. However, if new functionalities are not needed, the user can still continue using the same old functions without having to modify their code since the old set of functions are backwards compatible.

In Table \@ref(tab:old-vs-new-functions) it is shown the old and new set of standard functions for waveform generation. Functions in the left and right columns generate exactly the same output.

Table: 

|             | Old functions | New functions |
|-------------|---------------|---------------|
| +,x polarizations in Time/Fourier domain | XLALSimInspiralChooseTD(FD)Waveform | XLALSimInspiralGenerateTD(FD)Waveform |
| +,x polarizations in Time/Fourier domain with conditioning for Fourier transforms | XLALSimInspiralTD(FD)| XLALSimInspiralGenerateTD(FD)Waveform |
| Individual modes in Time/Fourier domain | XLALSimInspiralChooseTD(FD)Modes | XLALSimInspiralGenerateTD(FD)Modes |
[comment]: {#tab:old-vs-new-functions caption="This is the table's caption"}


We shall now briefly review the old set of functions and spell out their specific features to better understand the difference with the new ones.

- `XLALSimInspiralChooseTDWaveform` generates the time domain polarizations for time domain models but also Fourier domain models by internally performing an inverse Fourier transform.
- `XLALSimInspiralChooseFDWaveform` generates the Fourier domain polarizations of only Fourier domain models. It does not support time domain models.
- `XLALSimInspiralTD` generates time domain polarizations both for time and Fourier domain models. When the model is in the time domain, it will condition it so that if a Fourier transform is applied, the result is sensible. In `XLALSimInspiralChooseTDWaveform` this conditioning is not applied. In the case of Fourier domain models, an inverse Fourier transform is applied and the result should agree with `XLALSimInspiralChooseTDWaveform`.
- `XLALSimInspiralFD` generates Fourier domain polarizations both for time and Fourier domain models. For time domain models it will perform an direct Fourier transform. For Fourier domain models, it only applies the conditioning so that if the waveform is transformed to the time domain the result will be sensible. 
- `XLALSimInspiralChooseTDModes` generates the individual time domain modes of a time domain model. There are no Fourier transforms or conditioning.
- `XLALSimInspiralChooseFDModes` generates the individual Fourier domain modes of a Fourier domain model. There are no Fourier transforms or conditioning.

In all the Fourier transforms applied above there is a conditioning step which prepares the waveform so that its transform produces sensible results. The new set of functions can perform the Fourier transforms internally and optionally apply the conditioning, thereby preventing the duplication of the number of functions.

The old set of functions requires a fixed list of input parameters, which are different between the polarizations and modes functions and that may have different names for the same parameter. The new functions instead require only two input arguments, which are the same for all the functions, meaning that with the same two objects, one can evaluate the whole set of new functions. The two arguments comprise: First, a dictionary-type (LALDict) object that contains all the physical arguments needed to generate the waveform (those explicitly passed to the old functions) and that provides a standarized naming convention. Second, a generator-type (LALSimInspiralGenerator) object which stores information about the approximant to be used. Both objects and their type are described in more detail later. Below we show an example comparing the input arguments for the old and new functions.

```
XLALSimInspiralChooseTDWaveform(                   XLALSimInspiralGenerateTDWaveform(
 **hplus,                                           **hplus,
 **hcross,                                          **hcross,
 m1, m2, S1x, S1y, S1z, S2x, S2y, S2z,              LALDict *parameters, 
 distance, inclination, phiRef,                     LALSimInspiralGenerator *generator 
 longAscNodes, eccentricity, meanPerAno,           )
 deltaT, f_min, f_ref, 
 LALDict *extra_parameters, 
 Approximant approximant
)
 ``` 

The type of hpluss and hcross is REAL8TimeSeries and the rest of arguments in the left function are floats (REAL8). The old functions remain backwards compatible, but they have been transformed into wrappers that employ the new set of functions under-the-hood.
 
Finally, both the new and old set of functions support external waveform models implemented either in C or in Python. The difference relies on that the old functions will require to instantiate the Python interpreter in each evaluation, making them very inefficient. The new ones instead, will store the interpreter in the generator object, and subsequent evaluations will reuse it and save computational time.

Next, we describe more deeply the first of the input arguments for the new set of functions. This is a LALDict object that store the names and values of the physical parameters needed to generate the waveform. Unlike the Python interface, all the arguments are assumed to be expressed in SI units. A list of the supported arguments, their standarized names and their units can be found in LALSimInspiralWaveformParams_common.c. (# Tell more about those parameters?) Each parameter is inserted in the LALDict object with a specific function. For example, the total_mass argument is inserted doing
```
LALDict *params = XLALCreateDict();
REAL8 total_mass_value = 50 * LAL_MSUN_SI;

XLALSimInspiralWaveformParamsInsertTotalMass(params, total_mass_value);
```
Similarly, the value can be read form the LALDict object with
```
REAL8 total_mass_value = XLALSimInspiralWaveformParamsLookupTotalMass(params);
```

All the Insert and Lookup functions are declared and defined in the files LALSimInspiralWaveformParams.h(c). The Lookup functions for the mass and spins arguments can internally compute the required parameter from other parameters in the dictionary when this is not explicitly present in the dictionary. For example, the `XLALSimInspiralWaveformParamsLookupMass1(2)` will derive the component masses as long as two mass arguments are present in the dictionary, and at least one of them is dimensional. Consequently, the only forbidden combination is mass_ratio - sym_mass_ratio. The combination chirp_mass - mass_difference does not allow an analytical computation of the component masses and a numerical solver is used instead. If the mass arguments in the dictionary are underspecified of overspecified (the number of mass arguments is lower or larger than 2), an error is raised. If the combination of mass arguments does not allow to distinguish the most massive object, then $m_1 > m_2$ is assumed. This situation happens if the combination does not include the mass_ratio or the mass_difference, since all the other parameters are symmetric for $m_1$, $m_2$. The Lookup functions for the other mass arguments will internally call the LookupMass1(2) and compute the value from the mass components according to the definitions in Table #FIXME.

The spins components can be specified either in cartesian or polar coordinates. One single spin cannot contain a mixture of polar and cartesian components, i.e. to define a spin we will use either the three cartesian components or the three polar ones.

In the new set of functions, if the arguments inclination, phi_ref, f22_ref or the spin components are not present in the dictionary, they will be assumed to be zero. If the rest of arguments are not present or cannot be computed, an error is raised.

Besides the required arguments for waveform generation, the LALDict object can include extra options like the mode array content, tidal or beyond GR parameters, and all the other extra options found in LALSimInspiralWaveformParams.c.

The LALDict object can be transformed to a Python dictionary and viceversa through the functions `from_lal_dict` and `to_lal_dict` in `gwsignal.core.utils`. #FIXME: does this support all the extra LALDict options?

| Parameter | Definition |
|-----------|------------|
| total_mass | $m_1 + m_2$ |
| mass_ratio | $m_2 / m_1$ |
| sym_mass_ratio | $\frac{m_1 m_2}{(m_1 + m_2)^2}$ |
| mass_difference | $m_1 - m_2$ |
| reduced_mass | $\frac{m_1 m_2}{m_1 + m_2} $ |
| chirp_mass | $\frac{(m_1 m_2)^{3/5}}{(m_1 + m_2)^{1/5}}$ |
| spin_norm | $\sqrt{\mathrm{spin\_x}^2 + \mathrm{spin\_y}^2 + \mathrm{spin\_z}^2}$ |
| spin_tilt | $\arccos(\mathrm{spin\_z} / \mathrm{spin\_norm})$ |
| spin_phi | $\texttt{atan2}(\mathrm{spin\_y}, \mathrm{spin\_x})$ #FIXME: fix the code, it is using atan of the quotient |


















