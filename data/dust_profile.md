# Analytic Flaring Disk Model

See [Chiang & Goldreich (1997)](https://ui.adsabs.harvard.edu/abs/1997ApJ...490..368C/abstract).

## Volumetric Mass Density

The simulation grid operates in spherical coordinates $(r, \theta, \phi)$. The dust mass density $\rho_d$ at a given radius $r$ and polar angle $\theta$ is formulated as a vertical Gaussian distribution centered on the midplane ($\theta = \pi/2$):

$$\rho_d(r, \theta) = \frac{\Sigma_d(r)}{\sqrt{2\pi} h(r)} \exp\left(-\frac{(\pi/2 - \theta)^2}{2 (h(r)/r)^2}\right)$$

## Physical Components

### 1. Surface Mass Density ($\Sigma_d$)

The surface density of the dust follows a power-law distribution:
$$\Sigma_d(r) = \Sigma_{d0} \left(\frac{r}{\text{AU}}\right)^{p_{\text{sig}}}$$

### 2. Pressure Scale Height ($h$)

The vertical scale height $h(r)$ relies on the assumption of hydrostatic equilibrium and is derived from the isothermal sound speed ($c_s$) and Keplerian frequency ($\Omega_k$):
$$h(r) = \frac{c_s(r)}{\Omega_k(r)}$$

- $c_s(r) = \sqrt{\frac{k_B T_{\text{mid}}(r)}{\mu m_p}}$, where $\mu = 2.3$.
- $\Omega_k(r) = \sqrt{\frac{G M_*}{r^3}}$.

### 3. Midplane Temperature ($T_{\text{mid}}$)

The disk is heated passively by the central star with a fixed grazing angle $\alpha$, assuming it intercepts a fraction of the total luminosity:
$$T_{\text{mid}}(r) = \left( \frac{F_{\text{irr}}(r)}{\sigma_{\text{SB}}} \right)^{1/4}$$

- $F_{\text{irr}}(r) = \alpha \frac{L_*}{4\pi r^2}$.

---

## Model Parameters

The specific variables that define this particular disk profile, as initialized in the setup script:

### Stellar Parameters

- **Stellar Mass ($M_*$):** $1.0 M_\odot$ (`1.98892e33` g)
- **Stellar Radius ($R_*$):** $1.0 R_\odot$ (`6.96e10` cm)
- **Stellar Temperature ($T_*$):** $5780$ K
- **Stellar Luminosity ($L_*$):** Calculated via $4\pi R_*^2 \sigma_{\text{SB}} T_*^4$ ($\approx 3.8525 \times 10^{33}$ erg/s)

### Disk Geometry & Properties

- **Radiative Incidence Angle ($\alpha$):** $0.05$ (Constant flaring angle)
- **Inner Radius ($R_{\text{in}}$):** $10$ AU
- **Outer Radius ($R_{\text{out}}$):** $100$ AU
- **Dust Surface Density at 1 AU ($\Sigma_{d0}$):** $0.1$ g/cm$^2$ (Derived from a gas density of 10 g/cm$^2$ and a 1% dust-to-gas ratio)
- **Surface Density Power-Law Index ($p_{\text{sig}}$):** $-1.0$
- **Mean Molecular Weight ($\mu$):** $2.3$
