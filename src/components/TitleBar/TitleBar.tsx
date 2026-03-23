import React from "react";
import "./TitleBar.css";

const TitleBar: React.FC = () => {
  const api = window.electronAPI;

  return (
    <div className="title-bar">
      <div className="title-bar-content">
        <div className="title-bar-title">Handwrite Software</div>
        <div className="title-bar-controls">
          <button
            type="button"
            className="control-btn"
            aria-label="Minimize"
            onClick={() => void api?.minimizeWindow?.()}
          >
            −
          </button>
          <button
            type="button"
            className="control-btn"
            aria-label="Maximize"
            onClick={() => void api?.maximizeWindow?.()}
          >
            □
          </button>
          <button
            type="button"
            className="control-btn close"
            aria-label="Close"
            onClick={() => void api?.closeWindow?.()}
          >
            ×
          </button>
        </div>
      </div>
    </div>
  );
};

export default TitleBar;
