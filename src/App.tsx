import React from "react";
import TitleBar from "./components/TitleBar/TitleBar.tsx";
import TopMenuBar from "./components/TopMenuBar/TopMenuBar.tsx";
import ImageCanvas from "./components/ImageCanvas/ImageCanvas.tsx";
import "./App.css";

const App: React.FC = () => {
  return (
    <div className="app">
      <TitleBar />
      <TopMenuBar />
      <main className="main-content">
        <ImageCanvas />
      </main>
    </div>
  );
};

export default App;
