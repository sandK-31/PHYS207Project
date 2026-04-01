FROM sandk33/ansg-base-env:arm64

WORKDIR /ansg

# Create environment setup script (NO foreground commands)
RUN echo '#!/bin/bash' > /ansg/env.sh \
    && echo 'source /opt/root/bin/thisroot.sh' >> /ansg/env.sh \
    && echo 'source /opt/geant4/bin/geant4.sh' >> /ansg/env.sh \
    && echo 'source /opt/ratpac-two/ratpac.sh' >> /ansg/env.sh \
    && echo '' >> /ansg/env.sh \
    && echo '# Start virtual display (if not already running)' >> /ansg/env.sh \
    && echo 'if ! pgrep Xvfb > /dev/null; then' >> /ansg/env.sh \
    && echo '    Xvfb :0 -screen 0 1920x1080x24 > /dev/null 2>&1 &' >> /ansg/env.sh \
    && echo 'fi' >> /ansg/env.sh \
    && echo 'export DISPLAY=:0' >> /ansg/env.sh \
    && echo 'export QT_QPA_PLATFORM=xcb' >> /ansg/env.sh \
    && echo '' >> /ansg/env.sh \
    && echo '# Start VNC server (if not already running)' >> /ansg/env.sh \
    && echo 'if ! pgrep x11vnc > /dev/null; then' >> /ansg/env.sh \
    && echo '    x11vnc -display :0 -rfbport 5900 -passwd 1234 -forever > /dev/null 2>&1 &' >> /ansg/env.sh \
    && echo 'fi' >> /ansg/env.sh \
    && chmod +x /ansg/env.sh

# -------------------------
# Proper entrypoint script
# -------------------------
RUN echo '#!/bin/bash' > /ansg/entrypoint.sh \
    && echo 'source /ansg/env.sh' >> /ansg/entrypoint.sh \
    && echo 'exec "$@"' >> /ansg/entrypoint.sh \
    && chmod +x /ansg/entrypoint.sh

# -------------------------
# Workspace and user setup
# -------------------------
WORKDIR /workspace
VOLUME /workspace

ENV HOME=/workspace
ENV LD_LIBRARY_PATH=/opt/geant4/lib:$LD_LIBRARY_PATH
ENV RATSHARE=/opt/ratpac-two

# -------------------------
# Entrypoint & default shell
# -------------------------
ENTRYPOINT ["/ansg/entrypoint.sh"]
CMD ["bash"]
