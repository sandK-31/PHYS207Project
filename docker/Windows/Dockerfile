FROM sandk33/ansg-base-env:latest

WORKDIR /ansg
RUN echo "source /opt/root/bin/thisroot.sh" >> /ansg/env.sh \
    && echo "source /opt/geant4/bin/geant4.sh" >> /ansg/env.sh \
    && echo "source /opt/ratpac-two/ratpac.sh" >> /ansg/env.sh

# -------------------------
# Workspace and user setup
# -------------------------
WORKDIR /workspace
VOLUME /workspace
ENV HOME=/workspace
ENV LD_LIBRARY_PATH=/opt/geant4/lib
ENV RATSHARE=/opt/ratpac-two

# -------------------------
# Entrypoint & CMD
# -------------------------
ENTRYPOINT ["/bin/bash", "-c", "source /ansg/env.sh && exec bash"]
